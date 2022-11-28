use strict;
use warnings;
use Digest::CRC;

my $ctx = Digest::CRC->new( type => 'crc32' );

open my $fh, '<:raw', $ARGV[0] or die $!;
$ctx->addfile(*$fh);
close $fh;

print $ctx->hexdigest, "\n";