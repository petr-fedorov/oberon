format 222

pseudostatecanvas 128006 pseudostate_ref 162054 // initial
   xyz 45.4898 45.0898 2000
end
statecanvas 129414 state_ref 157190 // Pristine
  
  xyzwh 131.987 85.4521 2005 155 63
end
statecanvas 138758 state_ref 163846 // Fills
  
  xyzwh 214.067 351.729 2013 117 33
end
statecanvas 139014 state_ref 163974 // Cancellations
  
  xyzwh 202.258 498.062 2013 140 33
end
statecanvas 139270 state_ref 164102 // Fills and Cancellations
  
  xyzwh 451.371 421.863 2019 151 33
end
statecanvas 140422 state_ref 164230 // Contaminated
  
  xyzwh 177.981 258.381 2000 546 386
end
pseudostatecanvas 151430 pseudostate_ref 228230 // final
   xyz 354.92 53.0895 2000
end
pseudostatecanvas 152070 pseudostate_ref 234758 // shallow_history
   xyz 462.699 342.611 2005
end
transitioncanvas 138886 transition_ref 206982 // <transition>
  decenter_begin 116
  
  from ref 129414 z 2014 label "filled" xyz 135 251 2014 to point 145.033 367.247
  line 149254 z 2014 to ref 138758
  write_horizontally default show_definition default drawing_language default
end
transitioncanvas 139142 transition_ref 207110 // <transition>
  decenter_begin 539
  
  from ref 129414 z 2014 label "cancelled" xyz 64 111 2014 to point 47.3444 116.87
  line 149126 z 2014 to point 45.1263 513.171
  line 148998 z 2014 to ref 139014
  write_horizontally default show_definition default drawing_language default
end
transitioncanvas 139398 transition_ref 207238 // <transition>
  
  from ref 138758 z 2020 label "cancelled / update contaminated period" xyz 294 393 2020 to ref 139270
  write_horizontally default show_definition default drawing_language default
end
transitioncanvas 139526 transition_ref 207366 // <transition>
  
  from ref 139014 z 2020 label "filled / update contaminated period" xyz 300 468 2020 to ref 139270
  write_horizontally default show_definition default drawing_language default
end
transitioncanvas 144902 transition_ref 209542 // <transition>
  decenter_begin 835
  decenter_end 100
  
  from ref 139014 z 2014 to point 316.684 566.952
  line 145030 z 2014 label "cancelled / update contaminated period" xyz 161 561 2014 to point 213.45 566.952
  line 145158 z 2014 to ref 139014
  write_horizontally default show_definition default drawing_language default
end
transitioncanvas 145286 transition_ref 209670 // <transition>
  decenter_begin 786
  decenter_end 170
  
  from ref 138758 z 2014 to point 304.375 428.9
  line 145414 z 2014 label "filled / update contaminated period" xyz 198 431 2014 to point 230.304 428.9
  line 145542 z 2014 to ref 138758
  write_horizontally default show_definition default drawing_language default
end
transitioncanvas 146054 transition_ref 209926 // <transition>
  
  from ref 128006 z 2006 to ref 129414
  write_horizontally default show_definition default drawing_language default
end
transitioncanvas 146950 transition_ref 307974 // <transition>
  decenter_end 539
  
  from ref 139270 z 2020 to point 798.256 429.009
  line 151174 z 2020 to point 797.147 110.216
  line 149510 z 2020 label "elapsed [wait duration exceeded]" xyz 454 108 2020 to ref 129414
  write_horizontally default show_definition default drawing_language default
end
transitioncanvas 147974 transition_ref 308230 // <transition>
  
  from ref 139270 z 2020 to point 471.59 585.024
  line 148102 z 2020 label "filled / update contaminated period" xyz 347 550 2020 to point 404.282 526.589
  line 148230 z 2020 to ref 139270
  write_horizontally default show_definition default drawing_language default
end
transitioncanvas 148614 transition_ref 308486 // <transition>
  
  from ref 139270 z 2020 to point 666.859 516.498
  line 148742 z 2020 label "cancelled / update contaminated period" xyz 534 545 2020 to point 568.17 569.279
  line 148870 z 2020 to ref 139270
  write_horizontally default show_definition default drawing_language default
end
transitioncanvas 149638 transition_ref 308614 // elapsed, leave state
  decenter_begin 153
  decenter_end 819
  
  from ref 140422 z 2006 label "elapsed [wait duration exceeded] / pass messages" xyz 124 193 2006 to ref 129414
  write_horizontally default show_definition default drawing_language default
end
transitioncanvas 151558 transition_ref 308998 // <transition>
  decenter_begin 142
  
  from ref 129414 z 2006 label "stop" xyz 308 73 2006 to ref 151430
  write_horizontally default show_definition default drawing_language default
end
transitioncanvas 151686 transition_ref 315526 // <transition>
  decenter_begin 877
  decenter_end 135
  
  from ref 129414 z 2006 to point 264.013 32.7084
  line 151814 z 2006 label "message / pass" xyz 165 27 2006 to point 147.251 32.7084
  line 151942 z 2006 to ref 129414
  write_horizontally default show_definition default drawing_language default
end
transitioncanvas 152198 transition_ref 322054 // message
  decenter_begin 736
  
  from ref 140422 z 2006 label "message / save message" xyz 499 285 2006 to point 555.752 326.775
  line 152582 z 2006 to ref 152070
  write_horizontally default show_definition default drawing_language default
end
transitioncanvas 152326 transition_ref 322182 // elapsed, ignored
  decenter_begin 158
  
  from ref 140422 z 2006 label "elapsed [wait duration NOT exceeded]" xyz 589 331 2006 to point 659.095 357.156
  line 152454 z 2006 to ref 152070
  write_horizontally default show_definition default drawing_language default
end
end
