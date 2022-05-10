---------------------------- MODULE Afek_Gafani ----------------------------

EXTENDS Integers, Sequences

CONSTANT nodes_number

VARIABLE nodes,
         ord_message_channels,
         cand_message_channels


vars==<<nodes,ord_message_channels,cand_message_channels>>

\* channel_number == 
\*     nodes_number \X nodes_number



message_generate(temp_level,temp_id,temp_sender) ==
    [  level |-> temp_level, id |-> temp_id, sender |->temp_sender]

RECURSIVE Combine_Queue(_,_)
Combine_Queue(number_left,messages) ==
    IF number_left=0
        THEN <<>>
    ELSE  Combine_Queue(number_left-1,messages) \o Tail(messages)

Remove_message(id,message_channels) ==
    \* [ m \in 1..Len(message_channels[id]) |-> message_channels[id][m] ]
    IF Len(message_channels[id])<=1
        THEN << >>
    ELSE Combine_Queue(Len(message_channels[id])-1,message_channels[id])

(*judge order of two msg,if msg_1 smaller, return -1, equal return 0, larger return 1*)
Compare_Order(msg_1,level,id) ==
    IF msg_1.level < level 
        THEN -1
    ELSE 
        IF msg_1.level>level
            THEN 1
        ELSE 
            IF msg_1.id<id
                THEN -1
            ELSE 0

Message_Initialize ==
    /\ ord_message_channels = [m \in nodes_number |-> << >> ]
    /\ cand_message_channels = [m \in nodes_number |-> << >>]

nodes_Initialize ==
    /\ nodes= [m \in nodes_number |-> 
                [cand |-> 
                    [
                        status |-> "send",  
                        level  |-> 0,
                        killed |-> 0,
                        elected |->0,
                        msg_dealing |-> <<0,0,0>>,
                        untraversed |->  nodes_number \{m}
                    ],
                 ord |->
                    [
                        status |->"receive",
                        owner_level |-> -1,
                        msg_dealing |-> <<0,0,0>>,
                        potential_owner|-> -1,
                        owner_id |-> -1,
                        owner |-> -1
                    ]
                ] 
            ]

Init ==
    /\Message_Initialize
    /\nodes_Initialize

(************************************************************************************)
(*spec to test*)
(************************************************************************************)

Candidate_status_OK ==
    \A id \in nodes_number: nodes[id].cand.status \in {"send","receive","judge_1_1","judge_1_2","judge_1_2_2","final_judge"}

Ordinary_status_OK ==
    \A id \in nodes_number: nodes[id].ord.status \in {"send","receive","judge_1_2","judge_1_2_1_1","judge_1_2_1_2","judge_1_3"}

TypeOK ==
    /\ Candidate_status_OK
    /\ Ordinary_status_OK


(************************************************************************************)
(*candidate part*)
(************************************************************************************)
candidate_send(from,to) == 
    /\ nodes[from].cand.status="send"
    /\ to \in nodes[from].cand.untraversed
    /\ LET temp_level==nodes[from].cand.level
           temp_id==from
           temp_sender==temp_id
        IN ord_message_channels'=[ord_message_channels EXCEPT ![to] = 
        Append(ord_message_channels[to],message_generate(temp_level,temp_id,temp_sender))]
    /\ nodes'=[nodes EXCEPT ![from].cand.status="receive"]
    /\ UNCHANGED cand_message_channels

candidate_receive(id) ==
    /\ nodes[id].cand.status="receive"
    /\ cand_message_channels[id] # <<>>
    /\ cand_message_channels'=[cand_message_channels EXCEPT ![id] = Remove_message (id,cand_message_channels)]
    /\ LET msg==Head(cand_message_channels[id])
        IN IF msg.id=id /\ nodes[id].cand.killed=0
                THEN nodes'=[nodes EXCEPT ![id].cand.msg_dealing=msg,
                                          ![id].cand.status="judge_1_1"]

            ELSE nodes'=[nodes EXCEPT ![id].cand.msg_dealing=msg,
                                      ![id].status="judge_1_2"]
    /\ UNCHANGED <<ord_message_channels>>
    \*         ELSE 
    \* nodes'=[nodes EXCEPT ![from].cand.status=""]


candidate_judge_1_1(id) ==
    /\ nodes[id].cand.status="judge_1_1"
    /\ LET msg==nodes[id].cand.msg_dealing
        IN nodes'=[nodes EXCEPT ![id].cand.level=nodes[id].cand.level+1,
                                ![id].cand.untraversed=nodes[id].cand.untraversed \ {msg.sender},
                                ![id].cand.status="final_judge"]
    /\ UNCHANGED <<cand_message_channels,ord_message_channels>>

candidate_judge_1_2(id) ==
    /\ nodes[id].cand.status="judge_1_2"
    /\ LET msg==nodes[id].cand.msg_dealing
           level==nodes[id].cand.level
        IN IF Compare_Order(msg,level,id)=-1
                THEN nodes'=[nodes EXCEPT ![id].cand.status="receive"]
            ELSE 
                nodes'=[nodes EXCEPT ![id].cand.status="judge_1_2_2"]
    /\ UNCHANGED <<cand_message_channels,ord_message_channels>>

candidate_judge_1_2_2(id) ==
    /\ nodes[id].cand.status="judge_1_2_2"
    /\ LET msg==nodes[id].cand.msg_dealing
        IN  LET temp_level==msg.level
                temp_id==msg.id
                temp_sender==id
                to==msg.sender
            IN ord_message_channels'=
                [ord_message_channels EXCEPT ![to] = Append(ord_message_channels[to],message_generate(temp_level,temp_id,temp_sender))]
    /\ nodes'=[nodes EXCEPT ![id].cand.killed=1,
                            ![id].cand.status="receive"]
    /\ UNCHANGED <<cand_message_channels>>

(*the last step in While loop*)
candidate_final_judge(id) ==
    /\ nodes[id].cand.status="final_judge"
    /\ IF nodes[id].cand.killed=0
        THEN nodes'=[nodes EXCEPT ![id].cand.status="send",
                                  ![id].cand.elected=1]
        ELSE nodes'=[nodes EXCEPT ![id].cand.status="receive"]
    /\ UNCHANGED <<cand_message_channels,ord_message_channels>>

(************************************************************************************)
(*ordinary part*)
(************************************************************************************)


ordinary_receive(id) ==
    /\ nodes[id].ord.status="receive"
    /\ ord_message_channels[id] # <<>>
    /\ ord_message_channels'= [ord_message_channels EXCEPT ![id] = Remove_message(id,ord_message_channels)]
    /\ LET msg==Head(ord_message_channels[id]) 
           temp_level==nodes[id].ord.owner_level
           temp_id==nodes[id].ord.owner_id
        IN 
            CASE (Compare_Order(msg,temp_level,temp_id)) = -1 ->
                nodes'=[nodes EXCEPT ![id].ord.status="receive"]
            [] (Compare_Order(msg,temp_level,temp_id)) = 1 ->
                nodes'=[nodes EXCEPT ![id].ord.status="judge_1_2",
                                    ![id].ord.msg_dealing=msg]
            [] (Compare_Order(msg,temp_level,temp_id)) = 0 ->
                nodes'=[nodes EXCEPT ![id].ord.status="judge_1_3",
                                    ![id].ord.msg_dealing=msg]
    /\ UNCHANGED cand_message_channels

ordinary_judge_1_2(id)==
    /\ nodes[id].ord.status="judge_1_2"
    /\ IF nodes[id].ord.owner=-1
            THEN nodes'=[nodes EXCEPT ![id].ord.status="judge_1_2_1_1"]
        ELSE nodes'=[nodes EXCEPT ![id].ord.status="judge_1_2_1_2"]
    /\ UNCHANGED <<cand_message_channels,ord_message_channels>>

ordinary_judge_1_2_1_1(id)==
    /\nodes[id].ord.status="judge_1_2_1_1"
    /\ LET msg==nodes[id].ord.msg_dealing
        IN  LET temp_level==msg.level
                temp_id==msg.id
                temp_sender==id
                to==msg.sender
            IN  nodes'= [ nodes EXCEPT  ![id].ord.status="send",
                                        ![id].ord.owner=temp_sender,
                                        ![id].ord.potential_owner=temp_sender,
                                        ![id].ord.owner_level=temp_level,
                                        ![id].ord.owner_id=temp_id]
    /\ UNCHANGED <<cand_message_channels,ord_message_channels>>

ordinary_judge_1_2_1_2(id)==
    /\nodes[id].ord.status="judge_1_2_1_2"
    /\ LET msg==nodes[id].ord.msg_dealing
        IN  LET temp_level==msg.level
                temp_id==msg.id
                temp_sender==id
                to==msg.sender
            IN  nodes'= [ nodes EXCEPT  ![id].ord.status="send",
                                        ![id].ord.potential_owner=temp_sender,
                                        ![id].ord.owner_level=temp_level,
                                        ![id].ord.owner_id=temp_id]
    /\ UNCHANGED <<cand_message_channels,ord_message_channels>>

ordinary_judge_1_3(id)==
    /\ nodes[id].ord.status="judge_1_3"
    /\ nodes'=[nodes EXCEPT ![id].ord.status="send",
                            ![id].ord.owner=nodes[id].ord.potential_owner]
    /\ UNCHANGED <<cand_message_channels,ord_message_channels>>

ordinary_send(id)==
    /\ nodes[id].ord.status="send"
    /\ LET msg==nodes[id].ord.msg_dealing
           to==nodes[id].ord.owner_id
        IN  LET temp_level==msg.level
                temp_id==msg.id
                temp_sender==id
            IN  cand_message_channels'=[cand_message_channels EXCEPT ![to] = 
        Append(cand_message_channels[to],message_generate(temp_level,temp_id,temp_sender))]
    /\ nodes'=[nodes EXCEPT ![id].ord.status="receive"]
    /\ UNCHANGED ord_message_channels            


(************************************************************************************)
(*Next State*)
(************************************************************************************)


candidateNext ==
    \/ \E from \in nodes_number: \E to \in nodes_number \ {from} : candidate_send(from,to)
    \/ \E id \in nodes_number:  \/ candidate_receive(id)
                                \/ candidate_judge_1_1(id)
                                \/ candidate_judge_1_2(id)
                                \/ candidate_judge_1_2_2(id)
                                \/ candidate_final_judge(id)

ordinaryNext==
    \/ \E id \in nodes_number:  \/ordinary_send(id)
                                \/ordinary_receive(id)
                                \/ordinary_judge_1_2(id)
                                \/ordinary_judge_1_2_1_1(id)
                                \/ordinary_judge_1_2_1_2(id)
                                \/ordinary_judge_1_3(id)

Next ==
    \/ candidateNext
    \/ ordinaryNext
    \* \/ \E r \in nodes_number: ordinary_receive(r)

(************************************************************************************)
(*Spec Test*)
(************************************************************************************)

\*None_Elected ==
\*    \A id \in nodes_number: nodes[id].cand.eleceted=0
\*
\*
\*Keep_Going ==
\*    Init /\ [] [Next]_vars
\*
\*THEOREM Keep_Going => None_Elected


=============================================================================
\* Modification History
\* Last modified Fri Jan 22 16:59:45 CET 2021 by xuan
\* Created Sat Jan 09 15:30:46 CET 2021 by xuan
