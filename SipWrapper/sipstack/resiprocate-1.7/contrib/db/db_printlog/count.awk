# $Id: count.awk,v 1.1.1.1 2011/07/05 04:44:45 ivanzh Exp $
#
# Print out the number of log records for transactions that we
# encountered.

/^\[/{
	if ($5 != 0)
		print $5
}
