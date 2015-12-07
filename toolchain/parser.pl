#!/usr/bin/perl
if(scalar@ARGV < 1)
{
    die "usage:", "<input dot file>";
}
open(INFILE, "$ARGV[0]");
open(OUTFILE, ">$ARGV[0].ir");
$line = <INFILE>;
$line = <INFILE>;
%id;    # hash id by node name
%op;    # hash op by node name
@src;   
@dst;
$cnt = 0;
$cnt2 = 0;
while($line = <INFILE>)
{
    if($line =~ /(label)/) # op
    {
        #multiple matching
        #(@tokens) = $line  =~ m/\b\w+\b/g;
        @tokens = split /\s+/, $line;
        if( $tokens[2] =~ /add|sub/i)
        {
            $id{$tokens[0]} = $cnt;
            $op{$tokens[0]} = "add";
            $cnt++;
        }
        elsif( $tokens[2] =~ /mul/i)
        {
            $id{$tokens[0]} = $cnt;
            $op{$tokens[0]} = "mul";
            $cnt++;
        }
    }
    elsif($line =~/\-\>/)
    {
        @tokens = split /\s+/, $line;
        #print $tokens[0], $tokens[1], "\n";
        $src[$cnt2] = $tokens[0];
        $dst[$cnt2] = $tokens[2];
        $cnt2 ++;
    }
}
$num_op = keys%id;
print OUTFILE $num_op, "\n";
foreach my$i (keys%id)
{
    print OUTFILE $id{$i}, " ", $op{$i}, "\n";
}
foreach my$i (0..($cnt2-1))
{
    if(defined $id{$src[$i]} and defined $id{$dst[$i]})
    {
        print OUTFILE $id{$src[$i]}, " ", $id{$dst[$i]}, "\n";
    }
}
