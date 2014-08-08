# This PERL script converts assembler written in ARM syntax
# to assembler in GNU syntax.
# It is not complete, but should provide a basis for something
# more complete.
#
#
$end = "";
$file = shift;
$outfile = shift;
if (!$file) { die "Must specify file"; }
if (!$outfile) { $outfile = "$file.S";}
print "Input = $file\n";
print "Output = $outfile\n";
open (FILEB,$file) || die "$file not found";
open (OUTFILE,">$outfile") || die "Can't open output file $outfile";
while (<FILEB>) {
	chomp;
#   print "[".$_."]";
#   if( m!\S! ) { print "\n"; } else { print "<== empty line.\n"; }
    print "Unexpected code at line $. after END.\n" if ($end && $_ && m/\S/ );
    if (m!^\|?[a-zA-Z0-9_]\|?!) {
	m!^(\|?[a-zA-Z0-9_]*\|?)(\s*)([^;]*)!;
	$label = $1;
	$spacing1 = $2;
	$operation = $3;
	$comment = $';
	$label =~ s/\|//g;
#   print "***",$label,"\n";
    } else {
	m!^(\s*)([^;]*)!;
	$label = "";
	$spacing1 = $1;
	$operation = $2;
	$comment = $'
    }

    $comment =~ s/^;/@/;

    if(!$operation) {
	#print OUTFILE;
        $label = "$label:" if ($label);
	print OUTFILE "$label$spacing1$comment\n";
	next;
    }

    $operation =~ m/^(\S*)(\s*)/;
    $operator = $1;
    $spacing2 = $2;
    $operands = $';
    $spacing3 = "";
    while(($a = chop($operands)) =~ m/\s/) {
	$spacing3 = "$spacing3$a";
    }
    $operands = "$operands$a";

# must be done before label is hacked!
    if($operator =~ m/^(equ|[*])$/i) {
        $b = fix_expr($operands);
        if ($b =~ m/(\\)/) {
            $flag_continuation = 1;
            $b =~ s/(\\)//;
            print OUTFILE "$spacing1.equ$spacing2$label,$b";
        } else {
            $flag_continuation = 0;
            print OUTFILE "$spacing1.equ$spacing2$label,$b$spacing3$comment\n";
        }
    	next
    }

    $label = "$label:" if ($label);
    if ($operator =~ m/^END$/) {
	$end = "END";
#	print "\@END seen\n";
	print OUTFILE "$label$spacing1$comment\n";
	next;
    }

    if ($operator =~ m/^FUNCTION$/) {
	print OUTFILE "\@NOT SUPPORTED: $label$spacing1$operator$spacing2$operands$spacing3$comment\n";
	print OUTFILE "$label\n";
	next;
    }

    if ($operator =~ m/^ENDFUNC$/) {
	print OUTFILE "\@NOT SUPPORTED: $label$spacing1$operator$spacing2$operands$spacing3$comment\n";
	next;
    }

    if ($flag_macro == 1) {
        if ($operands =~ m/\$/g) {
            $operands =~ s/\$/\\/g;
        }
    }

    if($operator =~ m/^ttl$/i) {
	print OUTFILE "$label$spacing1.title$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^subt$/i) {
	print OUTFILE "$label$spacing1.sbttl$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(include|get)$/i) {
	# need to convert the operand file name to match the munging done by makegnuassembly.pl
	$newoperands = $operands;
        if ($operands =~ m/\.mac$/i)            # name ends with 4 chars .mac?
        {
        	$newoperands =~ s/\.mac$/_linux.MAC/i;
        }
	elsif ($operands =~ m/\.inc$/i)         # name ends with 4 chars .inc?
	{
		$newoperands =~ s/\.inc$/_linux.INC/i;
	}
	elsif($operands =~ m/\._s$/i)           # name ends with 3 chars ._s?
	{
		$newoperands =~ s/\._s$/_linux._S/i;
	}
	elsif($operands =~ m/\.s$/i)            # name ends with 2 chars .s?
	{
		$newoperands =~ s/\.s$/_linux.S/i;
	}

	print OUTFILE "$label$spacing1.include$spacing2\"$newoperands\"$spacing3$comment\n";
	#print "$label$spacing1.include$spacing2\"$newoperands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(cmp|cmn|tst|teq)(EQ|NE|CS|HS|CC|LO|MI|PL|VS|VC|HI|LS|GE|LT|GT|LE|AL|NV)?$/i) {
	print OUTFILE "$label$spacing1$operator$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(add|adc|and|bic|eor|mla|mov|mul|mvn|orr|rsb|rsc|sbc|smlal|smull|sub|umlal|umull)(EQ|NE|CS|HS|CC|LO|MI|PL|VS|VC|HI|LS|GE|LT|GT|LE|AL|NV)?S?$/i) {
    $a = fix_expr($operands);
	print OUTFILE "$label$spacing1$operator$spacing2$a$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(adr)|(adrl)|(ldfd)|(ldfs)|(ldr)|(nop)$/i) {
    $a = fix_expr($operands);
	print OUTFILE "$label$spacing1$operator$spacing2$a$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(b|bl)(EQ|NE|CS|HS|CC|LO|MI|PL|VS|VC|HI|LS|GE|LT|GT|LE|AL|NV)?$/i) {
    $operands =~ s/%F//g;
    $operands =~ s/%B//g;
	print OUTFILE "$label$spacing1$operator$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(swi)(EQ|NE|CS|HS|CC|LO|MI|PL|VS|VC|HI|LS|GE|LT|GT|LE|AL|NV)?$/i) {
	print OUTFILE "$label$spacing1$operator$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(mrs|msr)(EQ|NE|CS|HS|CC|LO|MI|PL|VS|VC|HI|LS|GE|LT|GT|LE|AL|NV)?$/i) {
	print OUTFILE "$label$spacing1$operator$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(ldr|str)(EQ|NE|CS|HS|CC|LO|MI|PL|VS|VC|HI|LS|GE|LT|GT|LE|AL|NV)?(b|t|bt|H|SB|SH)?$/i) {
    $a = fix_expr($operands);
	print OUTFILE "$label$spacing1$operator$spacing2$a$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(ldm|stm)(EQ|NE|CS|HS|CC|LO|MI|PL|VS|VC|HI|LS|GE|LT|GT|LE|AL|NV)?(ia|ib|da|db|fd|fa|ed|ea)?$/i) {
	print OUTFILE "$label$spacing1$operator$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(swp)(EQ|NE|CS|HS|CC|LO|MI|PL|VS|VC|HI|LS|GE|LT|GT|LE|AL|NV)?(b)?$/i) {
	print OUTFILE "$label$spacing1$operator$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(cdp|ldc|mcr|mrc|stc)(EQ|NE|CS|HS|CC|LO|MI|PL|VS|VC|HI|LS|GE|LT|GT|LE|AL|NV)?$/i) {
	print OUTFILE "$label$spacing1$operator$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^[%]$/i) {
	print OUTFILE "$label$spacing1.space$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^area$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^code16$/i) {
	print OUTFILE "$label$spacing.code 16$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^code32$/i) {
	print OUTFILE "$label$spacing1.code 32$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^entry$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^nofp$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^#$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(\^|MAP)$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^rout$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if (($operator =~ m/^align$/i) && ($operands =~ m/^32$/)) {
    $operands = 5;
	print OUTFILE "$label$spacing1.align$spacing2$operands$spacing3$comment\n";
	next
    }

    if (($operator =~ m/^align$/i) && ($operands =~ m/^16$/)) {
    $operands = 4;
	print OUTFILE "$label$spacing1.align$spacing2$operands$spacing3$comment\n";
	next
    }

    if (($operator =~ m/^align$/i) && ($operands =~ m/^8$/)) {
    $operands = 3;
	print OUTFILE "$label$spacing1.align$spacing2$operands$spacing3$comment\n";
	next
    }

    if (($operator =~ m/^align$/i) && ($operands =~ m/^4$/)) {
    $operands = 2;
	print OUTFILE "$label$spacing1.align$spacing2$operands$spacing3$comment\n";
	next
    }

    if (($operator =~ m/^align$/i) && ($operands =~ m/^2$/)) {
    $operands = 1;
	print OUTFILE "$label$spacing1.align$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^data$/i) {
	print OUTFILE "$label$spacing1\@Thumb only - NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(\=|dcb)$/i) {
	$str = "$operands$spacing3$comment";
#	while ($str && $str =~ s!^("([^\"]*(\\.)*)*"|[^,;]*)(,\s*|\s*$|\s*;)!!) {
	while ($str && $str =~ s!^("([^\"]*(\\.))*"|[^,;]*)(,\s*|\s*$|\s*;)!!) {
		$a = $1;
		$lastmatch = $&;
		if ($lastmatch =~ m!^"!) {
		    print OUTFILE "$label$spacing1.ascii $a";
		} else {
		    $b = &fix_expr($a);
		    print OUTFILE "$label$spacing1.byte $b";
		}
		$label = "";
		if ($lastmatch =~ m!;$!){
		    print OUTFILE "$spacing3@$str\n";
		    last;
		} else {
		    print OUTFILE "\n";
	        }
	}
	#print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(\&|dcd)$/i) {
	print OUTFILE "$spacing1.align 2\n";
        $b = &fix_expr($operands);
	print OUTFILE "$label$spacing1.word$spacing2$b$spacing3$comment\n";
	next
    }

    if($operator =~ m/^dcdu$/i) {
        $b = &fix_expr($operands);
	print OUTFILE "$label$spacing1.word$spacing2$b$spacing3$comment\n";
	next
    }

    if($operator =~ m/^dcfd$/i) {
	print OUTFILE "$spacing1.align 2\n";
        $b = &fix_expr($operands);
	print OUTFILE "$label$spacing1.double$spacing2$b$spacing3$comment\n";
	next
    }

    if($operator =~ m/^dcfdu$/i) {
        $b = &fix_expr($operands);
	print OUTFILE "$label$spacing1.double$spacing2$b$spacing3$comment\n";
	next
    }

    if($operator =~ m/^dcfs$/i) {
        $b = &fix_expr($operands);
	print OUTFILE "$spacing1.align 2\n";
	print OUTFILE "$label$spacing1.single$spacing2$b$spacing3$comment\n";
	#print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^dcfsu$/i) {
        $b = &fix_expr($operands);
	print OUTFILE "$label$spacing1.single$spacing2$b$spacing3$comment\n";
	next
    }

    if($operator =~ m/^dcw$/i) {
        $b = &fix_expr($operands);
	print OUTFILE "$spacing1.align 1\n";
	print OUTFILE "$label$spacing1.short$spacing2$b$spacing3$comment\n";
	next
    }

    if($operator =~ m/^dcwu$/i) {
        $b = &fix_expr($operands);
	print OUTFILE "$label$spacing1.short$spacing2$b$spacing3$comment\n";
	next
    }

    if($operator =~ m/^ltorg$/i) {
	print OUTFILE "$label$spacing1.ltorg$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^assert$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^info$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^opt$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^cn$/i) {
	chop $label;
	$operands = "C$operands" if (!($operands =~ m/^c/i));
	print OUTFILE "#define $label$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^cp$/i) {
	chop $label;
	$operands = "P$operands" if (!($operands =~ m/^p/i));
	print OUTFILE "#define $label$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(export|global)$/i) {
	$operands =~ m/^(\S*)(\s*)/;
	$symbol = $1;
	$qualifiers = $';
	if($qualifiers) {
		print "Qualifiers \"$qualifiers\" for $operator at line $. are not supported\n";
	}
	print OUTFILE "$label$spacing1.global $symbol$spacing2$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(import|extern)$/i) {
	$operands =~ m/^(\S*)(\s*)/;
	$symbol = $1;
	$qualifiers = $';
	if($qualifiers) {
		print "Qualifiers \"$qualifiers\" for $operator at line $. are not supported\n";
	}
	print OUTFILE "$label$spacing1.extern $symbol$spacing2$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(fn)$/i) {
	chop $label;
	$operands = "F$operands" if (!($operands =~ m/^f/i));
	print OUTFILE "#define $label$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(gbla)$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(gbll)$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(gbls)$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(lcla)$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(lcll)$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(lcls)$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(seta)$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(setl)$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(sets)$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(keep)$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(rlist)$/i) {
	chop $label;
	print OUTFILE "#define $label$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(rn)$/i) {
	chop $label;
	print OUTFILE "$label$spacing2.req$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(if)$/i)
    {
        if(($operands =~ m/:LNOT:/) && ($operands =~ m/:DEF:/)  && ($operands =~ m/:LOR:/))
        {
            $operands =~ s/:LNOT://;
            $operands =~ s/:DEF://;
            $operator =~ s/IF/.if/;
            $operands =~ s/ //;
			print OUTFILE "$operands\n";
        }
        else
        {
        	if(($operands =~ m/:LNOT:/) && ($operands =~ m/:DEF:/))
        	{
	            $operands =~ s/:LNOT://;
	            $operands =~ s/:DEF://;
	            $operator =~ s/IF/.ifndef/;
	            $operands =~ s/ //;
	        }
	        else
	        {
	            if($operands =~ m/:DEF:/)
	            {
	                $operands =~ s/:DEF://;
	                $operator =~ s/IF/.ifdef/;
	                $operands =~ s/ //;
	            }
	            else
	            {
	                if($operands =~ m/=0/)
	                {
	                    $operands =~ s/=0//;
	                    $operator =~ s/IF/.ifeq/;
	                    $operands =~ s/ //;
	                }
	                else
	                {
	                    if($operands =~ m/=1/)
	                    {
	                        $operands =~ s/=1//;
	                        $operator =~ s/IF/.ifne/;
	                        $operands =~ s/ //;
	                    }
	                    else
	                    {
	                        $operator =~ s/IF/.if/;
							$operands =~ s/:LOR:/ || /;
		                }
		           }
        		}
			}
		}
        print OUTFILE "$label$spacing1$operator$spacing2$operands$spacing3$comment\n";
    	next
    }

    if($operator =~ m/^(def)$/i) {
	$operator =~ s/def/.equ/;
	print OUTFILE "$label$spacing1$operator$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(\||else)$/i) {
  	print OUTFILE "$label$spacing1\.else$spacing2$operands$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(\]|endif)$/i) {
	print OUTFILE "$label$spacing1.endif$spacing2$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(incbin)$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(macro)$/i) {
        $flag_start_macro = 1;
        $operator =~ s/MACRO/.macro/;
        print OUTFILE "$label$spacing1$operator$spacing2$operands$spacing3$comment";
    	next
    }

    if($operator =~ m/^(mend)$/i) {
        $flag_macro = 0;
        $operator =~ s/MEND/.endm/;
    	print OUTFILE "$label$spacing1$operator$spacing2$operands$spacing3$comment";
    	next
    }

    if($operator =~ m/^(mexit)$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(wend)$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if($operator =~ m/^(while)$/i) {
	print OUTFILE "$label$spacing1\@NOT SUPPORTED: $operator$spacing2\"$operands\"$spacing3$comment\n";
	next
    }

    if ($flag_start_macro == 1) {
        $flag_start_macro = 0;
        $flag_macro = 1;
        $operands =~ s/\$//g;
        $operands =~ s/ //g;
        print OUTFILE "$label$spacing1$operator$spacing2$operands$spacing3$comment\n";
        next
    } else {
        if ($flag_continuation == 1) {
            $a = fix_expr($operator);
            $a =~ s/ //;
            $b = fix_expr($operands);
            $b =~ s/\\//;
            print OUTFILE "$a$b";
            next
        } else {
            print OUTFILE "$label$spacing1$operator$spacing2$operands$spacing3$comment\n";
            next
        }
    }
}
close(FILEB);
close(OUTFILE);

sub fix_expr {
    local($expression) = @_;
    $expression =~ s/(:OR:)/|/ig;
    $expression =~ s/(:AND:)/&/ig;
    $expression =~ s/(:EOR:)/^/ig;
    $expression =~ s/(:MOD:)/%/ig;
    $expression =~ s/(:SHL:)/<</ig;
    $expression =~ s/(:SHR:)/>>/ig;
#    print "$expression\n";
    return $expression;
}
