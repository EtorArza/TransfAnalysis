#!/usr/bin/perl -w
#
# Copyright (C) 2001-2011 J.I. van Hemert
#
# gdbscan.pl - a density-based clustering algorithm
#
# This software implements the algorithm described in:
# J. Sander, M. Ester, H.-P. Kriegel and X. Xu. Density-Based Clustering in
# Spatial Databases: The Algorithm GDBSCAN and Its Applications. In Data Mining
# and Knowledge Discovery 2, p. 169-194 (1998). Kluwer Academic Publishers (The
# Netherlands).
#
# This software is licensed under the Academic Free License version 3.0
# 
# Please refer to the full license included in LICENSE.txt
#
# Author: J.I. van Hemert <jano@vanhemert.co.uk>
# $Id: gdbscan.pl,v 1.2 2010/07/15 13:42:10 vanheme Exp $
#

use Getopt::Long;
use strict;
my $version = "1.0.5";

#################
# Configuration #
#################
my $config_MinCard = 4;
my $config_minimal_distance = 1.0;
my $config_delimiter = "[ ,	]+";
my $config_npred = "euclidean";
my %npredicates = (	"euclidean" => \&NPred_Euclidean,
			"binary" => \&NPred_binary
		);
my $flag_help = 0;
my $flag_verbose = 0;
Getopt::Long::Configure("permute", "no_ignore_case");
GetOptions(	"mincard|m=n" => \$config_MinCard,
		"e|eps=f" => \$config_minimal_distance,
		"d|delimiter=s" => \$config_delimiter,
		"predicate|p=s" => \$config_npred,
		"V|Verbose+" => \$flag_verbose,
		"h|help!" => \$flag_help
	);
if ($flag_help)
{
print STDERR "
gdbscan.pl $version copyright (c) 2001-2002 by J.I. van Hemert <jvhemert\@cs.leidenuniv.nl>
This software comes with ABSOLUTELY NO WARRANTY. It is free software and
you are welcome to redistribute it under the comditions of the GPL license.
See the source file for details.

Usage:
  -d, -delimiter    determines delimiter(s) for input:          \"$config_delimiter\"
  -e, -eps          minimum distance in predicate (gdbscan):    \"$config_minimal_distance\"
  -m, -mincard      minimal cardinality (gdbscan):              \"$config_MinCard\"
  -p, -predicate    name of predicate function (gdbscan):       \"$config_npred\"
  -V, -Verbose      increases verbose level every time added:   $flag_verbose
  -h, -help         this: version and help
\n";
	exit(0);
}


###############
# Read STDIN. #
###############
my @points;
my $largest_object_size = 0;
my $number_of_objects = 0;
while (<>)
{
	chomp $_;
	my @line = split $config_delimiter, $_;
	
	if ($_)
	{
		if ($largest_object_size < @line)
		{
			$largest_object_size = @line;
		}
		$number_of_objects++;
		push (@points, {data => \@line});
	}
}

print STDERR "largest object size $largest_object_size\n" if ($flag_verbose);
print STDERR "objects read: $number_of_objects\n" if ($flag_verbose);

gdbscan(\@points, $npredicates{$config_npred}, $config_MinCard, \&wCard);

############################################
# Output data with last column cluster id. #
############################################
print STDERR "                     \r" if ($flag_verbose);
my %clusters;
my $outliers = 0;
foreach my $i (@points)
{
	print join(' ',@{$i->{data}});
	if (defined $i->{cluster})
	{
		if ($i->{cluster})
		{
			print " $i->{cluster}\n";
			$clusters{$i->{cluster}}->{COUNT} += 1;
			my $a = 0;
			foreach my $e ( @{$i->{data}} )
			{
				$clusters{$i->{cluster}}->{CENTROIDS}->{$a} += $e;
				$a++;
			}
		}
		else
		{
			print " 0 noise\n";
			$outliers++;
		}
	}
	else
	{
		print " undef!\n";
	}
}

print STDERR "OUTLIERS $outliers\n";
printf STDERR "CLUSTERS %i\n", scalar keys %clusters;
foreach my $c (sort {$a <=> $b} keys %clusters)
{
	printf STDERR "CLUSTER-%i %i", $c, $clusters{$c}->{COUNT};
	foreach my $v ( sort {$a <=> $b} keys %{$clusters{$c}->{CENTROIDS}} )
	{
		printf STDERR " %g", $clusters{$c}->{CENTROIDS}->{$v}/$clusters{$c}->{COUNT};
	}
	print STDERR "\n";
}

#########################
# Main algorithm' loop. #
#########################
sub gdbscan
{
	my @objects = @{$_[0]};
	my @params = ($_[1], $_[2], $_[3]);
	
	my $cluster_id = 1; # 0 means noise
	my $objects_counter = $number_of_objects;
	foreach my $object (@objects)
	{
		print STDERR "objects: $objects_counter \r" if ($flag_verbose);
		unless (defined $object->{cluster})
		{
			print STDERR "main: $object->{data}->[0] $object->{data}->[1]\n" if ($flag_verbose > 1);
			if (expandcluster(\@points, $object, $cluster_id, @params))
			{
				$cluster_id++;
				print STDERR "\n" if ($flag_verbose);
			}
			print STDERR "\n" if ($flag_verbose > 1);
		}
		$objects_counter--;
	}
}

#####################################################
# Create a cluster from a given object and return 1 #
# OR return 0 if the object is not a core object.   #
#####################################################
sub expandcluster
{
	my @objects = @{$_[0]};
	my $object = $_[1];
	my $cluster_id = $_[2];
	my $npred = $_[3];
	my $MinCard = $_[4];
	my $wcard = $_[5];

	my @seeds = neighborhood(\@objects, $object, $npred);

	foreach my $i (@seeds)
	{
		print STDERR "neigh: $i->{data}->[0] $i->{data}->[1]\n" if ($flag_verbose > 1);
	}
	print STDERR "card: ".&$wcard(\@seeds)."\n" if ($flag_verbose > 1);
	if (&$wcard(\@seeds) < $MinCard)
	{
		$object->{cluster} = 0;
		print STDERR "not core\n" if ($flag_verbose > 1);
		return 0;
	}
	print STDERR "core\n" if ($flag_verbose > 1);

	my $objects_counter = 0;
	while (@seeds)
	{
		print STDERR "cluster: $cluster_id objects: $objects_counter seeds: $#seeds \r" if ($flag_verbose);
		my $current_object = shift @seeds;
		unless ($object == $current_object)
		{
			my @result = neighborhood(\@objects, $current_object, $npred);
			if (wCard(\@result) >= $MinCard)
			{
				foreach my $r (@result)
				{
					if ( !defined($r->{cluster}) or ($r->{cluster} == 0)  )
					{
						if ( !defined($r->{cluster}) )
						{
							push @seeds, $r;
						}
						$r->{cluster} = $cluster_id;
						$objects_counter++;
					}
				}
			}
		}
	}
	
	return 1;
}


###########################################
# Create a list of neighbours using NPred #
# for an object.                          #
###########################################
sub neighborhood
{
	my @objects = @{$_[0]};
	my $object = $_[1];
	my $npred = $_[2];

	my @result;
	foreach my $i (@objects)
	{
		if (&$npred($object, $i))
		{
			push @result, $i;
		}
	}
	return @result;
}

#####################################################
# NPred returns 1 if a certain predicate (distance) #
# is met OTHERWISE it returns 0.                    #
#####################################################
sub NPred_Euclidean
{
	my $object1 = shift;
	my $object2 = shift;

	my $distance = 0;
	#print STDERR "object_size=%i\n", scalar @{$object1->{data}};
	my $object_size = scalar @{$object1->{data}};
	for (my $i = 0; $i < $object_size; $i++)
	{
		$distance += ( $object1->{data}->[$i] - $object2->{data}->[$i] ) ** 2
	}

	if (sqrt($distance) < $config_minimal_distance)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
sub NPred_binary
{
	my $object1 = shift;
	my $object2 = shift;

	my $distance = 0;
	my $object_size = $#{@{$object1->{data}}};
	for (my $i = 0; $i <= $object_size; $i++)
	{
		if ( $object1->{data}->[$i] ne $object2->{data}->[$i] )
		{
			$distance++;
		}
	}

	if ($distance < $config_minimal_distance)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

#################################################
# Return cardinality of a set (here: an array). #
#################################################
sub wCard
{
	my @set = @{$_[0]};

	return ($#set + 1);
}

######################################
# Given a set of objects set every   #
# objects' cluster id to a given id. #
######################################
sub changeclusterid
{
	my @set = @{$_[0]};
	my $cluster_id = $_[1];

	foreach my $i (@set)
	{
		$i->{cluster} = $cluster_id;
	}
}
