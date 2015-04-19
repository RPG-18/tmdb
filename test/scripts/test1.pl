#!/usr/bin/perl

use 5.014;
use strict;
use warnings;
use Time::HiRes qw( usleep );
use IO::Socket::INET;

$| = 1;

my $socket = IO::Socket::INET->new(PeerAddr=>"localhost",
                                  PeerPort=>$ARGV[0],
                                  Proto => 'tcp')
                                  or die "ERROR in Socket Creation : $!\n";
$socket->autoflush(1);


my $startTime = 1400000000;
my $measCount = 300;
my $timeStep  = 300; # 5min;

my $time = $startTime;
for(my $i=0; $i<$measCount; ++$i)
{
	my $message = "add mymeas $time 15\n";
	print $message;
	print $socket $message;
	$time+=$timeStep;
}
print $socket "exit\n";

$socket = IO::Socket::INET->new(PeerAddr=>"localhost",
                                  PeerPort=>$ARGV[0],
                                  Proto => 'tcp')
                                  or die "ERROR in Socket Creation : $!\n";
                                  
print $socket "get_sequence mymeas 1400060000 1400079800\n";
while(<$socket>)
{
	chomp;
	last if($_ eq "done");
	say $_;
}
print $socket "exit\n";

