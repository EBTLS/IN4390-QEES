----------------------------- MODULE Question_2 -----------------------------

EXTENDS Integers


CONSTANT state_number

VARIABLES  state,
           q_3_counter

(***********************)

Init == 
    /\ state = [ r \in state_number |-> IF r=1 THEN 1
                                        ELSE 0]
    /\ q_3_counter=0


q_3_detect ==
    IF state= <<0,1,10>> 
            THEN q_3_counter'= q_3_counter+1
        ELSE q_3_counter'= q_3_counter


tA ==
    /\ state[1]>=1
    /\ state'=[state EXCEPT ![2]=state[2]+1,
                            ![1]=state[1]-1]
    /\ q_3_detect

tB ==
    /\ state[1]>=1
    /\ state'=[state EXCEPT ![3]=state[3]+1,
                            ![1]=state[1]-1]
    /\ q_3_detect

tC ==
    /\ state[2]>=1
    /\ state'=[state EXCEPT ![3]=state[3]+1]
    /\ q_3_detect

tD ==
    /\ state[3]>=1
    /\ state'=[state EXCEPT ![3]=state[3]-1,
                            ![2]=state[2]+1]
    /\ q_3_detect


Next ==
    \/ tA
    \/ tB
    \/ tC
    \/ tD
(*******************************)

q2 == 
    /\ state[2]<=state[3]

q3_1==
    /\ state # <<0,1,10>>

q3_2==
    /\ q_3_counter<2


\*q3_2==
\*    (state = <<0,1,1000>>) ~
\*    (state= <<1,0,0>>) ~> (state # <<0,1,3>>)
    
=============================================================================
\* Modification History
\* Last modified Tue Jan 12 09:29:52 CET 2021 by xuan
\* Created Fri Jan 08 22:29:18 CET 2021 by xuan
