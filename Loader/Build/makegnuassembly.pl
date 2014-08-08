#!/usr/bin/perl

$debug=1;	# $debug enables / disables debug features

print "makegnuassembly.pl ", @ARGV, ": enter.\n" if $debug; 

foreach $dir (@ARGV)
{

	opendir DH, $dir or die "Cannot open $dir: $!";

	foreach $file (readdir DH) {

		# check for directory names first...
		
		if( $file eq "." )  {next;} # if this is the current directory, just skip it.
		if( $file eq ".." ) {next;} # if this is the parent directory, just skip it.

		if( -d $dir.'/'.$file )     # if this is any other directory, recurse into it.
		{ 
			print "  dir: ", 1,  " file: ", $dir.'/'.$file, "   ***   recursing ...\n" if $debug;
			system "perl makegnuassembly.pl $dir/$file";
			next;
		};

		#
		# it's not a directory. see if the file name matches the assembly file category
		#
		
		$newfile = $file;
		$isassembly = 0;

		if ($file =~ m/\.mac$/)		# name ends with 4 chars .mac?
		{
			$newfile =~ s/\.mac$/_linux.MAC/;
			$isassembly = 1;
		}
		if ($file =~ m/\.inc$/)		# name ends with 4 chars .inc?
		{
			$newfile =~ s/\.inc$/_linux.INC/;
			$isassembly = 1;
		}
		if($file =~ m/\._s$/)		# name ends with 3 chars ._s?
		{
			$newfile =~ s/\._s$/_linux._S/;
			$isassembly = 1;
		}
		elsif($file =~ m/\.s$/)		# name ends with 2 chars .s?
		{
			$newfile =~ s/\.s$/_linux.S/;
			$isassembly = 1;
		}

		print "  asm: ", $isassembly,  " file: ", $dir.'/'.$file, "   ***   ", $newfile, "  ...\n" if $debug;

		#
		# file name examination complete.
		# # if the file name matched the assembly file category, convert it.
		#
		if($isassembly == 1)
		{
			system "perl convert_arm_asm_2gnu.pl $dir//$file $dir//$newfile"
		}
	}

	closedir DH;
}

if($debug == 1) { print "makegnuassembly.pl ", @ARGV, ": exit.\n"; }

