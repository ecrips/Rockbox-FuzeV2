#!/usr/bin/perl
#             __________               __   ___.
#   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
#   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
#   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
#   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
#                     \/            \/     \/    \/            \/
# $Id$
#

# addtargetdir.pl - Adds target directory to gcc-generated dependency data

use File::Basename;

my $rbroot = $ARGV[0];
my $builddir = $ARGV[1];

my $target2;

for (<STDIN>) {
    if (/^([^:]+): (\S+) (.*)/) {
        my ($target, $src, $rest) = ($1, $2, $3);
        my $dir = dirname $src;
        $dir =~ s/^.*$rbroot//;
        print "$builddir$dir/$target: $src $rest\n";
    }
    elsif (/^([^:]+):  \\/) {
        # target and source on different lines
        $target2 = $1;
    }
    elsif ($target2) {
        if (/^\s+([^ ]+) (.*)/) {
            my ($src, $rest) = ($1, $2);
            my $dir = dirname $src;
            $dir =~ s/^.*$rbroot//;
            print "$builddir$dir/$target2: $src $rest\n";
            $target2 = "";
        }
    }
    else {
        print $_;
    }
}
