# $Id: commit.awk,v 1.1.1.1 2011/07/05 04:44:45 ivanzh Exp $
#
# Output tid of committed transactions.

/txn_regop/ {
	print $5
}
