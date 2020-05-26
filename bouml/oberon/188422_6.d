format 222

activitycanvas 128006 activity_ref 155142 // S.5 Receive
  
  xyzwh 84 53 0 643 557
  params
    parametercanvas 128134 parameter_ref 156422 // websocket
        xyzwh 46 121 1 77 25
    end
    parametercanvas 128262 parameter_ref 156550 // message
        xyzwh 688 284 1 77 25 label_xy 678 319
    end
  end
end
activityactioncanvas 128390 activityaction_ref 192134 // activity action S.5 Wait for next message
  
  show_opaque_action_definition default
  xyzwh 445 272 3010 100 60
  pins
    pincanvas 130822 pin_ref 221702 // target
      xyzwh 493 262 3012 11 11 label_xy 454 253
    end
    pincanvas 130950 pin_ref 221830 // 
      xyzwh 544 291 3012 11 11 label_xy 555 316
    end
    pincanvas 131078 pin_ref 221958 // 
      xyzwh 435 315 3012 11 11 label_xy 299 319
    end
    pincanvas 131206 pin_ref 222086 // 
      xyzwh 435 292 3012 11 11 label_xy 330 261
    end
  end
end
activityobjectcanvas 129158 activityobject_ref 134534 // activity object S.5 websocket
  
  xyzwh 193 106 3010 99 59
end
note 130566 "\"tokens are always offered on the outgoing flows of a DataStoreNode,\"

See 15.4.3.4 Data Store Nodes"
  xyzwh 348 72 3010 259 79
activityactioncanvas 131590 activityaction_ref 200070 // activity action S.5 Disconnect
  
  show_opaque_action_definition default
  xyzwh 218 254 2000 100 60
  pins
    pincanvas 131718 pin_ref 222214 // target
      xyzwh 272 244 2002 11 11 label_xy 233 235
    end
  end
end
activityactioncanvas 132230 activityaction_ref 200198 // activity action S.5 Reconnect
  
  show_opaque_action_definition default
  xyzwh 118 393 2000 100 60
  pins
    pincanvas 132358 pin_ref 222342 // target
      xyzwh 148 383 2002 11 11 label_xy 109 374
    end
    pincanvas 132486 pin_ref 222470 // isConnected
      xyzwh 217 413 2002 11 11 label_xy 227 399
    end
  end
end
activitynodecanvas 133638 activitynode_ref 196614 // decision
  xyz 350 400 2000
end
note 134918 "Whether to wait before reconnect or not, is decided within Reconnect and depends on the known reason of disconnect. For example,  ExceptionDisconnected may contain information about  exchange's downtime"
  xyzwh 102 510 2000 329 77
note 135174 "Some logic may be plugged in here  later"
  xyzwh 391 450 2000 135 47
note 136198 "Generates an \"elapsed\" message when exchange time changes or time passes without messages from an exchange or heartbeat received"
  xyzwh 479 159 2000 233 79
flowcanvas 129286 flow_ref 217478 // S.5.1
  
  from ref 128134 z 3011 label "S.5.1" xyz 142 126 3011 to ref 129158
   write_horizontally default
end
flowcanvas 131334 flow_ref 226950 // S.5.2
  
  from ref 129158 z 3013 label "S.5.2" xyz 377 204 3013 to ref 130822
   write_horizontally default
end
flowcanvas 131846 flow_ref 227078 // S.5.6
  
  from ref 129158 z 3011 label "S.5.6" xyz 248 196 3011 to ref 131718
   write_horizontally default
end
flowcanvas 131974 flow_ref 227206 // S.5.4
  
  from ref 131206 z 3013 label "S.5.4" xyz 361 285 3013 to ref 131590
   write_horizontally default
end
flowcanvas 132614 flow_ref 227334 // S.5.7
  
  from ref 129158 z 3011 label "S.5.7" xyz 177 266 3011 to ref 132358
   write_horizontally default
end
flowcanvas 133766 flow_ref 227462 // S.5.8
  
  from ref 131590 z 2001 label "S.5.8" xyz 200 346 2001 to ref 132230
   write_horizontally default
end
flowcanvas 133894 flow_ref 227590 // S.5.9
  
  from ref 132486 z 2003 label "S.5.9" xyz 274 412 2003 to ref 133638
   write_horizontally default
end
flowcanvas 134022 flow_ref 227718 // S.5.11
  
  from ref 133638 z 2001 label "S.5.11
[false]" xyz 343 449 2001 to point 358 486
  line 134278 z 2001 to ref 132230
   write_horizontally default
end
flowcanvas 134534 flow_ref 227846 // S.5.10
  
  from ref 133638 z 3011 label "S.5.10
[true]" xyz 417 405 3011 to point 494 417
  line 134662 z 3011 to ref 128390
   write_horizontally default
end
flowcanvas 134790 flow_ref 227974 // S.5.3
  
  from ref 130950 z 3013 label "S.5.3" xyz 606 290 3013 to ref 128262
   write_horizontally default
end
flowcanvas 135814 flow_ref 228102 // S.5.5
  
  from ref 131078 z 3013 label "S.5.5" xyz 405 347 3013 to point 402 380
  line 135942 z 3013 to point 254 367
  line 136070 z 3013 to ref 132230
   write_horizontally default
end
line 131462 -_-_
  from ref 130566 z 3011 to ref 129158
line 135046 -_-_
  from ref 134918 z 2001 to ref 132230
line 135302 -_-_
  from ref 135174 z 2002 to ref 134022
line 136326 -_-_
  from ref 136198 z 3011 to ref 128390
end
