SipWrapper
========================================
SipWrapper is wrapper codes for reSIProcate stack, follows Apache2 license.


#### Why need SipWrapper?
ReSIProcate stack is an cpp style SIP stack implementation. But reSIProcate does not work well as bellow. So we add an wrapper for this stack to overcome these disadvantages.
<ul>
    <li>SIP API is not unified.</li>
    <li>ReSIProcate does not have higher SIP element abstract.</li>
</ul>


#### How we design?
We use old style network stack design pattern, first is layer design. Basically, there are three layers here. First is service provider layer, aimed for explore APIs to user. Second is call management layer, aimed for manage calls we hold, the defination of the call is the same as RFC3261. Third is adapter layer, aimed for porting SipWrapper to kinds of SIP stacks.

Bisically, the center of second layer is "call", which is represent for SIP session. For the call, we support call management, call state machine, call life management and so on.

We add some usefull utils like log, timer.
