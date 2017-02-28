### python, c style

1. every file is a code orginization, to explore:
- APIs and macro by header file.
- global variable like:
    -- global used variable like 'netdev chains'.
    -- global API holders like 'netdev ops[]'.
    -- so this file could be treated like one class, 1) and 2) is data member, 2) is data function member. And 1 is function member.
- internal used APIs/MACROs and variables:
    -- 'static' function and variables.
