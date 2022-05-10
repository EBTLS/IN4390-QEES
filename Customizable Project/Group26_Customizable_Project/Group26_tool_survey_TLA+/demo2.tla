------------------------- MODULE toolsurveyexample -------------------------


EXTENDS Integers


CONSTANT state_number

VARIABLES  state

(***********************)

Init == 
    /\ state = [ r \in state_number |-> IF r = 9 THEN 1
                                                 ELSE 0]





Register_order ==
        /\ state[9] >= 1
        /\ state'=[state EXCEPT ![0] = state[0]+1,
                                ![2] = state[2]+1,
                                ![9] = state[9]-1]


Check_availability == 
        /\ state[0] >= 1
        /\ state'=[state EXCEPT ![0] = state[0]-1,
                                ![1] = state[1]+1]

No_stock ==
        /\ state[1] >= 1
        /\ state'=[state EXCEPT ![1] = state[1]-1,
                                ![4] = state[4]+1]

Replenish ==
        /\ state[4] >= 1
        /\ state'=[state EXCEPT ![4] = state[4]-1,
                                ![3] = state[3]+1]

No_stock_no_replenishment ==
        /\ state[1] >= 1
        /\ state'=[state EXCEPT ![1] = state[1]-1,
                                ![3] = state[3]+1]

Update_database ==
        /\ state[3] >= 1
        /\ state'=[state EXCEPT ![3] = state[3]-1,
                                ![0] = state[0]+1]

In_stock ==
        /\ state[1] >= 1
        /\ state'=[state EXCEPT ![1] = state[1]-1,
                                ![5] = state[5]+1]


Stock_check == 
        \/ Check_availability
        \/ No_stock
        \/ Replenish
        \/ No_stock_no_replenishment
        \/ Update_database
        \/ In_stock


Deliver ==
        /\ state[5] >= 1
        /\ state'=[state EXCEPT ![5] = state[5]-1,
                                ![7] = state[7]+1]

Send_bill ==
        /\ state[2] >= 1
        /\ state'=[state EXCEPT ![2] = state[2]-1,
                                ![6] = state[6]+1]

Send_payment_reminder ==
        /\ state[6] >= 1
        /\ state[7] >= 1
        /\ state'=[state EXCEPT ![6] = state[6]-1,
                                ![7] = state[7]-1,
                                ![2] = state[2]+1]

Receive_payment ==
        /\ state[6] >= 1
        /\ state'=[state EXCEPT ![6] = state[6]-1,
                                ![8] = state[8]+1]


Payment ==
        \/ Send_bill
        \/ Send_payment_reminder
        \/ Receive_payment


Archive ==
        /\ state[7] >= 1
        /\ state[8] >= 1
        /\ state'=[state EXCEPT ![7] = state[7]-1,
                                ![8] = state[8]-1,
                                ![9] = state[9]+1]



Next ==
    \/ Register_order
    \/ Stock_check
    \/ Deliver
    \/ Payment
    \/ Archive


q2 == 
    /\ state[2]<=state[3]
(*
tA ==
    /\ state[1]>=1
    /\ state'=[state EXCEPT ![2]=state[2]+1,
                            ![1]=state[1]-1]

tB ==
    /\ state[1]>=1
    /\ state'=[state EXCEPT ![3]=state[3]+1,
                            ![1]=state[1]-1]

tC ==
    /\ state[2]>=1
    /\ state'=[state EXCEPT ![3]=state[3]+1]

tD ==
    /\ state[3]>=1
    /\ state'=[state EXCEPT ![3]=state[3]-1,
                            ![2]=state[2]+1]

Next ==
    \/ tA
    \/ tB
    \/ tC
    \/ tD
(*******************************)

q2 == 
    /\ state[2]<=state[3]

q3_1==
    /\ state # <<0,1,1000>>
    
    
*)    
    
\*    (state= <<1,0,0>>) ~> (state # <<0,1,3>>)

\*q3_2==
\*    (state = <<0,1,3>>) ~> (state = <<0,1,3>>)
\*q3_2==
\*    /\ (state = <<0,1,1000>>) 
\*    /\ (state' = <<0,1,1000>>)

=============================================================================
\* Modification History
\* Last modified Tue Jan 12 10:36:04 CET 2021 by LYD
\* Created Sat Jan 09 10:49:50 CET 2021 by LYD
