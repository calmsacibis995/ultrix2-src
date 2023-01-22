
#ifndef lint
static char sccsid[]  =  "@(#)dsd_initld.c	1.4   (ULTRIX)   1/29/87";
#endif  lint
#include  "generic_dsd.h"
#include  "std_dsd.h"
#include  "os_dsd.h"


/****************** CODED_ITEM_STRUCT *****************/

DD$STD_CODE  sc_ostype[] = { 
  {1,"es$vms","VMS"},
  {2,"es$ultrix32","ULTRIX 32"},
  {3,"es$tops_10","TOPS-10"},
  {4,"es$tops_20","TOPS-20"} };
DD$STD_CODE  sc_eventclass[] = { 
  {1,"ec$error","ERROR EVENT"},
  {2,"ec$operator","OPERATIONAL EVENT"},
  {3,"ec$maintenance","MAINTENANCE EVENT"},
  {4,"ec$statistical","STATISTICAL EVENT"},
  {5,"ec$corrupt","CORRUPTED EVENT"} };
DD$STD_CODE  sc_devclass[] = { 
  {1,"dc$cpu","CPU"},
  {2,"dc$memory","MEMORY"},
  {3,"dc$dsdisk","DSA DISK"},
  {4,"dc$mbdisk","MASSBUS DISK"},
  {5,"dc$dstape","DSA TAPE"},
  {6,"dc$mbtape","MASSBUS TAPE"},
  {7,"dc$bus","BUS"},
  {8,"dc$adapter","ADAPTER/CONTROLLER"},
  {9,"dc$comm","COMM DEVICE"},
  {10,"dc$card","CARD READER"},
  {11,"dc$terminal","TERMINAL"},
  {12,"dc$realtime","REALTIME"},
  {13,"dc$mailbox","MAILBOX/SOCKET"},
  {14,"dc$journal","JOURNAL"},
  {15,"dc$misc","UNKNOWN"},
  {16,"dc$floppy","FLOPPY DISK"},
  {17,"dc$ubdisk","UNIBUS DISK"},
  {18,"dc$ubtape","UNIBUS TAPE"},
  {19,"dc$rbdisk","RB CLASS DISK"},
  {20,"dc$tktape","TK CLASS TAPE"} };
DD$STD_CODE  sc_devtype[] = { 
  {1,"dt$8650","8650"},
  {2,"dt$8600","8600"},
  {3,"dt$8800","8800"},
  {4,"dt$8200","8200"},
  {5,"dt$785","785"},
  {6,"dt$780","780"},
  {7,"dt$750","750"},
  {8,"dt$uv1","MICROVAX I"},
  {9,"dt$uv2","MICROVAX II"},
  {10,"dt$aie","AIE CONTROLLER"},
  {11,"dt$aio"," AIO CONTROLLER"},
  {12,"dt$d78","DW780 ADAPTER"},
  {13,"dt$d50","DW750 ADAPTER"},
  {14,"dt$bia","BIA ADAPTER"},
  {15,"dt$bcci","CIBCI"},
  {16,"dt$78ci","CI780"},
  {17,"dt$75ci","CI750"},
  {18,"dt$r60","RA60"},
  {19,"dt$r80","RA80"},
  {20,"dt$r81","RA81"},
  {21,"dt$r82","RA82"},
  {22,"dt$rz1","RZ01"},
  {23,"dt$rzf01","RZF01"},
  {24,"dt$rb1","UNDEFINED"},
  {25,"dt$r31","RD31"},
  {26,"dt$r51","RD51"},
  {27,"dt$r52","RD52"},
  {28,"dt$r53","RD53"},
  {29,"dt$r54","RD54"},
  {30,"dt$rx5","RX50"},
  {31,"dt$rx3","RX33"},
  {32,"dt$ta78","TA78"},
  {33,"dt$ta81","TA81"},
  {34,"dt$tk50","TK50"},
  {35,"dt$tu81","TU81"},
  {36,"dt$rk6","RK06"},
  {37,"dt$rk7","RK07"},
  {38,"dt$rp4","RP04"},
  {39,"dt$rp5","RP05"},
  {40,"dt$rp6","RP06"},
  {41,"dt$rp7","RP07"},
  {42,"dt$r7ht","RP07HT"},
  {43,"dt$rl1","RL01"},
  {44,"dt$rl2","RL02"},
  {45,"dt$rx1","RX01"},
  {46,"dt$rx2","RX02"},
  {47,"dt$rx4","RX04"},
  {48,"dt$rm8","RM80"},
  {49,"dt$rm5","RM05"},
  {50,"dt$tu58","TU58"},
  {51,"dt$ml11","ML11"},
  {52,"dt$rb2","RB02"},
  {53,"dt$rb8","RB80"},
  {54,"dt$dz11","DZ11"},
  {55,"dt$dz32","DZ32"},
  {56,"dt$dz730","DZ730"},
  {57,"dt$dmc11","DMC11"},
  {58,"dt$dmr11","DMR11"},
  {59,"dt$u50","UDA50"},
  {60,"dt$u5a","UDA50A"},
  {61,"dt$t8p","TU81P"},
  {62,"dt$rc25","RC25"},
  {63,"dt$xk_3271","XK 3271"},
  {64,"dt$xu_2780","XU 2780"},
  {65,"dt$nw_x25","NW X25"},
  {66,"dt$nv_x29","NV X29"},
  {67,"dt$sb_isb11","SB ISB11"},
  {68,"dt$mx200","MUX200"},
  {69,"dt$dmp11","DMP11"},
  {70,"dt$dmf32","DMF32"},
  {71,"dt$xv_3271","XV 3271"},
  {72,"dt$ci","CI"},
  {73,"dt$ni","NI BUS"},
  {74,"dt$una11","UNA11"},
  {75,"dt$te16","TE16"},
  {76,"dt$tu45","TU45"},
  {77,"dt$t77","TU77"},
  {78,"dt$t78","TU78"},
  {79,"dt$ts11","TS11"},
  {80,"dt$t80","TU80"},
  {81,"dt$lax","LAX"},
  {82,"dt$yn_x25","YN X25"},
  {83,"dt$y0x25","Y0 X25"},
  {84,"dt$yp_adccp","YP ADCCP"},
  {85,"dt$y03271","Y0 3271"},
  {86,"dt$yr_ddcmp","DDCMP"},
  {87,"dt$yssdlc","YS SDLC"},
  {88,"dt$dr8","DR780"},
  {89,"dt$dr5","DR750"},
  {90,"dt$d1w","DR11W"},
  {91,"dt$p1r","PC 11R"},
  {92,"dt$p1t","PC 11T"},
  {93,"dt$d1c","DR11C"},
  {94,"dt$x1c","XI DR11C"},
  {95,"dt$xpb","XP PCL11B"},
  {96,"dt$uqprt","UQ PORT"},
  {97,"dt$rdx","RDRX"},
  {98,"dt$unknjnl","UNKNJNL"},
  {99,"dt$rujnl","RUJNL"},
  {100,"dt$bijnl","BIJNL"},
  {101,"dt$atjnl","ATJNL"},
  {102,"dt$aijnl","AIJNL"},
  {103,"dt$cljnl","CLJNL"},
  {104,"dt$cibca","CIBCA"},
  {105,"dt$uba","UBA"},
  {106,"dt$bua","BUA"},
  {107,"dt$nmi","NMI/BI ADAPTER"},
  {108,"dt$78sb","780 SBI"},
  {109,"dt$86sb","8600 SBI"},
  {110,"dt$bib","BI"},
  {111,"dt$nmb","NMI BUS"},
  {112,"dt$rcf25","RCF25"},
  {113,"dt$rrd50","RRD50"},
  {114,"dt$rv8","RV80"},
  {115,"dt$rx18","RX18"},
  {116,"dt$ra70","RA70"},
  {117,"dt$ra90","RA90"},
  {118,"dt$c78","MS780C"},
  {119,"dt$78e","MS780E"},
  {120,"dt$m75","MS750"},
  {121,"dt$m86","MS8600"},
  {122,"dt$m88","MS8800"},
  {123,"dt$m82","MS8200"},
  {124,"dt$muv2","MS630"},
  {125,"dt$bla","BLA CONTROLLER"},
  {126,"dt$rc25ctl","RC25 CONTROLLER"},
  {127,"dt$rux5ctl","RUX50 CONTROLLER"},
  {128,"dt$tk5ctl","TK50 CONTROLLER"},
  {129,"dt$tu81ctl","TU81 CONTROLLER"},
  {130,"dt$rqdxctl","RQDX CONTROLLER"},
  {131,"dt$kda5ctl","KDA50 CONTROLLER"},
  {132,"dt$tk7ctl","TK70 CONTROLLER"},
  {133,"dt$rv8ctl","RV80 CONTROLLER"},
  {134,"dt$rrd5ctl","RRD50 CONTROLLER"},
  {135,"dt$kdb5ctl","KDB50 CONTROLLER"},
  {136,"dt$r3qdxctl","RQDX3 CONTROLLER"},
  {137,"dt$hsc50","HSC50"},
  {138,"dt$vmsctl","VMS MSCP CONTROLLER"},
  {139,"dt$topsctl","TOPS-10/20 MSCP CTRLR"},
  {140,"dt$kfbtactl","KFBTA CONTROLLER"},
  {141,"dt$hsc70","HSC70"},
  {142,"dt$hsb50","HSB50"},
  {143,"dt$ultctl","ULTRIX MSCP CTRLR"},
  {144,"dt$vxstr","MS400"},
  {145,"dt$m73","MS730"},
  {146,"dt$m83","MS8300"},
  {147,"dt$m850","MS8500"},
  {148,"dt$m855","MS8550"},
  {149,"dt$m87","MS8700"},
  {150,"dt$730","730"},
  {151,"dt$8300","8300"},
  {152,"dt$8500","8500"},
  {153,"dt$8550","8550"},
  {154,"dt$8700","8700"},
  {155,"dt$acp","ACP CONTROLLER"},
  {156,"dt$shdwfx","SHADOWFAX GRAPHICS"} };
DD$STD_CODE  sc_coarsesyndrome[] = { 
  {1,"mck86","8600 MACHINE CHECK"},
  {2,"mck78","780 MACHINE CHECK"},
  {3,"mck75","750 MACHINE CHECK"},
  {4,"mckuv1","MICROVAX I MACHINE CHECK"},
  {5,"mckuv2","MICROVAX II MACHINE CHECK"},
  {6,"mck88","8800 MACHINE CHECK"},
  {7,"mck82","8200 MACHINE CHECK"},
  {8,"bier","BI BUS ERROR"},
  {9,"nmibus","NMI BUS ERROR"},
  {10,"ubaadp","UBA ADAPTER ERROR"},
  {11,"buaadp","BUA ADAPTER ERROR"},
  {12,"nmiadp","NMI ADAPTER ERROR"},
  {13,"scbint","SCB INTERRUPT"},
  {14,"uniint","UNIBUS INTERRUPT"},
  {15,"cte86","8600 CONSOLE TIMEOUT"},
  {16,"cte88","8800 CONSOLE TIMEOUT"},
  {17,"kerstk","KERNEL STACK DUMP"},
  {18,"intstk","INTERRUPT STACK DUMP"},
  {19,"usrstk","USER MODE STACK DUMP"},
  {20,"raf","RESERVED ADDRESS FAULT"},
  {21,"pif","PRIVELEGED INSTRUCTION FAULT"},
  {22,"rof","RESERVED OPERAND FAULT"},
  {23,"bpt","BREAKPOINT INSTRUCTION FAULT"},
  {24,"xfc","XFC INSTRUCTION FAULT"},
  {25,"syscall","SYSTEM CALL EXCEPTION/FAULT"},
  {26,"atflt","ARITHMETIC TRAP EXCEPTION/FAULT"},
  {27,"astflt","AST EXCEPTION/FAULT"},
  {28,"segflt","SEGMENTATION FAULT"},
  {29,"protect","PROTECTION FAULT"},
  {30,"trace","TRACE EXCEPTION/FAULT"},
  {31,"pgflt","PAGE FAULT"},
  {32,"pgtflt","PAGE TABLE FAULT"},
  {33,"mem_crd","MEMORY CRD ERROR"},
  {34,"mem_rds","MEMORY RDS ERROR"},
  {35,"mem_ctl","MEMORY CONTROLLER ERROR"},
  {36,"mem_wmask","MEMORY WRITE MASK ERROR"},
  {37,"sbi_err","SBI ERROR"},
  {38,"sbi_wtim","SBI WTIME ERROR"},
  {39,"sbi_alrt","SBI ALERT"},
  {40,"sbi_flt","SBI FAULT"},
  {41,"sbi_fail","SBI FAILURE"},
  {42,"cmpflt","COMPAT MODE FAULT"},
  {43,"cnterr","CONTROLLER ERROR"},
  {44,"hmeacc","MEMORY ACCESS ERROR"},
  {45,"dsktrerr","DISK TRANSFER ERROR"},
  {46,"sdierr","SDI ERROR"},
  {47,"smdskerr","SMALL DISK ERROR"},
  {48,"bdblkrepl","BAD BLK REPLC ATTMPT"},
  {49,"tptrerr","TAPE TRANSFER ERROR"},
  {50,"sticomm","STI COMMUNIC/CMD FAILURE"},
  {51,"stidrverr","STI DRIVE ERROR"},
  {52,"stifmterr","STI FORMATTER ERR"},
  {53,"uqattn","UQ PORT ATTENTION"},
  {54,"blaerr","BLA ERROR"},
  {55,"bvperr","BVP ERROR"},
  {56,"ciattn","CI ATTENTION MSG"},
  {57,"cilpkt","CI LOGGED PACKET"},
  {58,"mem_par","MEMORY PARITY ERROR"},
  {59,"mem_nxm","NON-EXISTANT MEMORY"},
  {60,"mck83","8300 MACHINE CHECK"},
  {61,"mck850","8500 MACHINE CHECK"},
  {62,"mck855","8550 MACHINE_CHECK"},
  {63,"mck87","8700 MACHINE CHECK"},
  {64,"mck73","730 MACHINE CHECK"} };
DD$STD_CODE  sc_errtyp[] = { 
  {1,"memcrd","CRD"},
  {2,"memrds","RDS"},
  {3,"memctl","CONTROLLER"},
  {4,"memwmsk","WMASK"} };

/*************** REG_FIELD_CODES_STRUCT **************/

DD$FIELD_CODE  fc_1[] = { 
  {100,"MACHINE CHECK"},
  {101,"MEMORY ERROR"},
  {102,"DISK ERROR"},
  {103,"TAPE ERROR"},
  {104,"CONTROLLER ERROR"},
  {105,"ADAPTER ERROR"},
  {106,"BUS ERROR"},
  {107,"STRAY INT"},
  {108,"ASYNCH WRITE ERR"},
  {109,"EXCEPTION/FAULT"},
  {110,"EMM EXCEPTION"},
  {111,"CONSOLE TIMEOUT"},
  {112,"STACK DUMP"},
  {200,"PANIC"},
  {250,"ASCII MSG"},
  {251,"SNAPSHOT LOGGED"},
  {300,"SYSTEM STARTUP"},
  {301,"SYSTEM SHUTDOWN"},
  {310,"TIME CHANGE"},
  {350,"DIAGNOSTIC MSG"},
  {351,"REPAIR MSG"} };
DD$FIELD_CODE  fc_2[] = { 
  {1,"FBOX DETECTED ERR"},
  {2,"EBOX DETECTED ERR"},
  {3,"IBOX DETECTED ERR"},
  {4,"MBOX DETECTED ERR"} };
DD$FIELD_CODE  fc_3[] = { 
  {1,"RESOURCE TURNED OFF"} };
DD$FIELD_CODE  fc_4[] = { 
  {1,"SBIA FULL RPT FOLLOWS"} };
DD$FIELD_CODE  fc_5[] = { 
  {1,"SBI SUMMARY EVENT VALID"} };
DD$FIELD_CODE  fc_6[] = { 
  {1,"MBOX INTERRUPT ENTRY VALID"} };
DD$FIELD_CODE  fc_7[] = { 
  {1,"PROCESS ABORT"} };
DD$FIELD_CODE  fc_8[] = { 
  {1,"MEAR SAVED"} };
DD$FIELD_CODE  fc_9[] = { 
  {1,"FIX IBOX CS PE"} };
DD$FIELD_CODE  fc_10[] = { 
  {1,"FIX IBOX DRAM PE"} };
DD$FIELD_CODE  fc_11[] = { 
  {1,"FIX FBOX DRAM PE"} };
DD$FIELD_CODE  fc_12[] = { 
  {1,"FIX FBA CS PE"} };
DD$FIELD_CODE  fc_13[] = { 
  {1,"FIX FBM CS PE"} };
DD$FIELD_CODE  fc_14[] = { 
  {1,"FIX IBOX GPR PE"} };
DD$FIELD_CODE  fc_15[] = { 
  {1,"FIX EBOX GPRA PE"} };
DD$FIELD_CODE  fc_16[] = { 
  {1,"FIX EBOX GPRB PE"} };
DD$FIELD_CODE  fc_17[] = { 
  {1,"FIX FBOX GPR PE"} };
DD$FIELD_CODE  fc_18[] = { 
  {1,"FBOX SERVC REQST"} };
DD$FIELD_CODE  fc_19[] = { 
  {1,"EHM ENTERED"} };
DD$FIELD_CODE  fc_20[] = { 
  {1,"MBOX SERVICE REQUEST"} };
DD$FIELD_CODE  fc_21[] = { 
  {1,"I/O READ"} };
DD$FIELD_CODE  fc_22[] = { 
  {1,"MEM/GPR READ"} };
DD$FIELD_CODE  fc_23[] = { 
  {1,"MACHINE STATE MODIFIED"} };
DD$FIELD_CODE  fc_24[] = { 
  {1,"EBOX ABORT"} };
DD$FIELD_CODE  fc_25[] = { 
  {1,"EBOX WBUS PE"} };
DD$FIELD_CODE  fc_26[] = { 
  {1,"EBOX DATA PATH PE"} };
DD$FIELD_CODE  fc_27[] = { 
  {1,"EBOX U-STK PE"} };
DD$FIELD_CODE  fc_28[] = { 
  {1,"EBOX CS PE"} };
DD$FIELD_CODE  fc_29[] = { 
  {1,"EBOX MCF RAM PE"} };
DD$FIELD_CODE  fc_30[] = { 
  {1,"IBOX ERROR"} };
DD$FIELD_CODE  fc_31[] = { 
  {1,"MBOX INTERRUPT PENDING"} };
DD$FIELD_CODE  fc_32[] = { 
  {1,"MBOX FATAL ERROR"} };
DD$FIELD_CODE  fc_33[] = { 
  {1,"PERFORM MONITOR ENABLED"} };
DD$FIELD_CODE  fc_34[] = { 
  {1,"IBOX CS CORRECTION REQUEST"} };
DD$FIELD_CODE  fc_35[] = { 
  {1,"IBOX DRAM CORRECTION REQUEST"} };
DD$FIELD_CODE  fc_36[] = { 
  {1,"FBOX DRAM CORRECTION REQUEST"} };
DD$FIELD_CODE  fc_37[] = { 
  {1,"FBA CS CORRECTION REQUEST"} };
DD$FIELD_CODE  fc_38[] = { 
  {1,"FBM CS CORRECTION REQUEST"} };
DD$FIELD_CODE  fc_39[] = { 
  {1,"EBOX GPRB PE"} };
DD$FIELD_CODE  fc_40[] = { 
  {1,"AMUX WBUS PE"} };
DD$FIELD_CODE  fc_41[] = { 
  {1,"EBOX GPRA PE"} };
DD$FIELD_CODE  fc_42[] = { 
  {1,"OPERAND PE"} };
DD$FIELD_CODE  fc_43[] = { 
  {1,"RESULT PE"} };
DD$FIELD_CODE  fc_44[] = { 
  {1,"BMUX OPBUS PE"} };
DD$FIELD_CODE  fc_45[] = { 
  {1,"BMUX WBUS PE"} };
DD$FIELD_CODE  fc_46[] = { 
  {1,"EDP MISC PE"} };
DD$FIELD_CODE  fc_47[] = { 
  {1,"WREG PE"} };
DD$FIELD_CODE  fc_48[] = { 
  {0,"IBOX REG SELECT"},
  {1,"EMD"},
  {2,"IBUFFER"},
  {3,"IMD OR ID REG"} };
DD$FIELD_CODE  fc_49[] = { 
  {1,"IMD REGISTER"},
  {0,"ID REGISTER"} };
DD$FIELD_CODE  fc_50[] = { 
  {0,"EBOX RD/WRT UTRAP"},
  {1,"OP WRITE UTRAP"},
  {2,"IBOX ERROR UTRAP"},
  {3,"MISC UTRAP"},
  {4,"FORK UTRAP"},
  {5,"IMD READ UTRAP"},
  {6,"ID READ UTRAP"},
  {7,"STRING RD UTRAP"} };
DD$FIELD_CODE  fc_51[] = { 
  {1,"ENABLE EBOX UTRAP LOGIC"} };
DD$FIELD_CODE  fc_52[] = { 
  {1,"IBOX CS PE"} };
DD$FIELD_CODE  fc_53[] = { 
  {1,"IBOX DRAM PE"} };
DD$FIELD_CODE  fc_54[] = { 
  {1,"IBOX AMUX PE"} };
DD$FIELD_CODE  fc_55[] = { 
  {1,"RLOG PE"} };
DD$FIELD_CODE  fc_56[] = { 
  {1,"INST BUFFER PE"} };
DD$FIELD_CODE  fc_57[] = { 
  {1,"IBOX BMUX PE"} };
DD$FIELD_CODE  fc_58[] = { 
  {1,"RESERVED MODE DETECTED"} };
DD$FIELD_CODE  fc_59[] = { 
  {0,"GPR"},
  {1,"WBUS"} };
DD$FIELD_CODE  fc_60[] = { 
  {1,"CACHE DATA PE DURING BYTE WRT"} };
DD$FIELD_CODE  fc_61[] = { 
  {1,"ARRAY READ"} };
DD$FIELD_CODE  fc_62[] = { 
  {1,"CACHE 1 SELECT"} };
DD$FIELD_CODE  fc_63[] = { 
  {1,"CACHE DATA PE"} };
DD$FIELD_CODE  fc_64[] = { 
  {1,"TB TAG PE"} };
DD$FIELD_CODE  fc_65[] = { 
  {1,"TB PTE A PE"} };
DD$FIELD_CODE  fc_66[] = { 
  {1,"TB PTE B PE"} };
DD$FIELD_CODE  fc_67[] = { 
  {1,"TB VALID PE"} };
DD$FIELD_CODE  fc_68[] = { 
  {1,"CACHE MISS"} };
DD$FIELD_CODE  fc_69[] = { 
  {1,"CACHE 0 TAG MISS"} };
DD$FIELD_CODE  fc_70[] = { 
  {1,"BLOCK HIT"} };
DD$FIELD_CODE  fc_71[] = { 
  {1,"TB MISS"} };
DD$FIELD_CODE  fc_72[] = { 
  {1,"ABUS CMD/ADDR CYCLE"} };
DD$FIELD_CODE  fc_73[] = { 
  {1,"ABUS ADDRESS PE"} };
DD$FIELD_CODE  fc_74[] = { 
  {1,"ABUS CMD OR MASK PE"} };
DD$FIELD_CODE  fc_75[] = { 
  {1,"ABUS DATA PE"} };
DD$FIELD_CODE  fc_76[] = { 
  {1,"CPR A PE"} };
DD$FIELD_CODE  fc_77[] = { 
  {1,"CPR B PE"} };
DD$FIELD_CODE  fc_78[] = { 
  {0,"NOP"},
  {1,"READ REG"},
  {2,"WRITE REG"},
  {3,"WRITEBACK"},
  {4,"ABUS ARRAY WRT"},
  {5,"DATA CORRECTION"},
  {6,"CLEAR CACHE"},
  {7,"TB PROBE"},
  {8,"ABUS"},
  {9,"CP REFILL"},
  {10,"INVALID TB"},
  {11,"TB CYCLE"},
  {12,"CP ARRAY WRT"},
  {13,"CP WRITE"},
  {14,"CP READ"},
  {15,"ABUS REFILL"} };
DD$FIELD_CODE  fc_79[] = { 
  {0,"IBUF"},
  {1,"IMD"},
  {2,"EMD"},
  {3,"IBUF"} };
DD$FIELD_CODE  fc_80[] = { 
  {1,"ABUS MEMORY LOCK"} };
DD$FIELD_CODE  fc_81[] = { 
  {1,"CP I/O BUFFER ERR"} };
DD$FIELD_CODE  fc_82[] = { 
  {1,"NXM"} };
DD$FIELD_CODE  fc_83[] = { 
  {1,"CACHE WBIT SET"} };
DD$FIELD_CODE  fc_84[] = { 
  {1,"CACHE WBIT PE"} };
DD$FIELD_CODE  fc_85[] = { 
  {1,"CACHE TAG PE"} };
DD$FIELD_CODE  fc_86[] = { 
  {1,"MULTIPLE ERR"} };
DD$FIELD_CODE  fc_87[] = { 
  {1,"ABUS BAD DATA"} };
DD$FIELD_CODE  fc_88[] = { 
  {1,"BYTE WRITE"} };
DD$FIELD_CODE  fc_89[] = { 
  {1,"ARRAY BUS LONGWORD PARITY  INVERTD"} };
DD$FIELD_CODE  fc_90[] = { 
  {1,"ABUS LONGWD PARITY INVERTED"} };
DD$FIELD_CODE  fc_91[] = { 
  {1,"ADDRESS PE"} };
DD$FIELD_CODE  fc_92[] = { 
  {1,"DOUBLE BIT ERR"} };
DD$FIELD_CODE  fc_93[] = { 
  {1,"SINGLE BIT ERR"} };
DD$FIELD_CODE  fc_94[] = { 
  {1,"BAD DATA SYNDROME DETECTED"} };
DD$FIELD_CODE  fc_95[] = { 
  {1,"WRITE CSL"},
  {0,"READ CSL"} };
DD$FIELD_CODE  fc_96[] = { 
  {1,"CBUS CLOCK ASSRTD"},
  {0,"CBUS CLOCK NOT ASSRTD"} };
DD$FIELD_CODE  fc_97[] = { 
  {1,"INTERRUPT SOURCE INTERNAL"},
  {0,"INTERRUPT SOURCE EXTERNAL"} };
DD$FIELD_CODE  fc_98[] = { 
  {1,"CONSOLE TRANSMIT  REQUESTED"} };
DD$FIELD_CODE  fc_99[] = { 
  {1,"CONSOLE RECEIVE  ACK"} };
DD$FIELD_CODE  fc_100[] = { 
  {1,"CONSOLE INTERRUPR  REQUEST FOR RL02"} };
DD$FIELD_CODE  fc_101[] = { 
  {1,"INTERVAL COUNT REG OVERFLOWED"} };
DD$FIELD_CODE  fc_102[] = { 
  {1,"MBOX ERROR"} };
DD$FIELD_CODE  fc_103[] = { 
  {1,"CPU POWER FAIL"} };
DD$FIELD_CODE  fc_104[] = { 
  {1,"CONSOLE HALT  PENDING"} };
DD$FIELD_CODE  fc_105[] = { 
  {1,"GENERATE EVEN PARITY ON  PHYSICAL ADDR"} };
DD$FIELD_CODE  fc_106[] = { 
  {1,"GENERATE EVEN PARITY ON CACHE TAG"} };
DD$FIELD_CODE  fc_107[] = { 
  {1,"GENERATE EVEN PARITY ON CACHE WBIT"} };
DD$FIELD_CODE  fc_108[] = { 
  {1,"GENERATE EVEN PARITY ON TB VALID"} };
DD$FIELD_CODE  fc_109[] = { 
  {1,"GENERATE EVEN PARITY ON TB TAG"} };
DD$FIELD_CODE  fc_110[] = { 
  {1,"ADDRESS TB RAM VIA DIAGNOSTIC"} };
DD$FIELD_CODE  fc_111[] = { 
  {1,"MEMORY MANAGEMENT ENABLED"} };
DD$FIELD_CODE  fc_112[] = { 
  {1,"IOA DIAGNOSTIC IN PRGRESS"} };
DD$FIELD_CODE  fc_113[] = { 
  {1,"INHIBIT ARRAY SBE REPORTS"} };
DD$FIELD_CODE  fc_114[] = { 
  {1,"GENERATE BAD ABUS CMD/MASK  PARITY"} };
DD$FIELD_CODE  fc_115[] = { 
  {1,"INHIBIT DMA REQUESTS"} };
DD$FIELD_CODE  fc_116[] = { 
  {0,"CACHE 0 & 1 DISABLED"},
  {1,"CACHE 1 DISABLED"},
  {2,"CACHE 0 DISABLED"},
  {3,"NORMAL RUNNING CODE"},
  {4,"CACHE 0 & 1 DISABLED"},
  {5,"FORCE SWAP OF CACHE 0 IF  WRITTEN BIT"},
  {6,"FORC SWAP OF CACHE 1 IF WRITTEN BIT"},
  {7,"CACHE 0 & 1 HIT"} };
DD$FIELD_CODE  fc_117[] = { 
  {1,"FORCE CACHE MISS"} };
DD$FIELD_CODE  fc_118[] = { 
  {1,"FBOX PROBLEM"} };
DD$FIELD_CODE  fc_119[] = { 
  {0,"F"},
  {1,"G"},
  {2,"D"},
  {3,"I OR H"} };
DD$FIELD_CODE  fc_120[] = { 
  {1,"RESERVED OPERAND FLT"} };
DD$FIELD_CODE  fc_121[] = { 
  {1,"FBOX GPR PE"} };
DD$FIELD_CODE  fc_122[] = { 
  {1,"SELF TEST ERR"} };
DD$FIELD_CODE  fc_123[] = { 
  {1,"FBOX DRAM PE"} };
DD$FIELD_CODE  fc_124[] = { 
  {1,"FBA CONTROL STORE PE"} };
DD$FIELD_CODE  fc_125[] = { 
  {1,"FBM CONTROL STORE PE"} };
DD$FIELD_CODE  fc_126[] = { 
  {0,"NORMAL"},
  {1,"OVERFLOW"},
  {2,"UNDERFLOW"},
  {3,"UNDERFLOW"} };
DD$FIELD_CODE  fc_127[] = { 
  {1,"EBOX CONTROL STORE PE"},
  {2,"IBOX CONTROL STORE PE"},
  {3,"IBOX DRAM PE"},
  {4,"FBOX DRAM PE"},
  {5,"FBOX ADDR MOD CS PE"},
  {6,"FBOX MULT MOD CS PE"},
  {7,"MBOX CONTROL STORE ERRS"} };
DD$FIELD_CODE  fc_128[] = { 
  {1,"CONSOLE DRAM CORRECTION ATTEMPT FAILED"} };
DD$FIELD_CODE  fc_129[] = { 
  {1,"TRACE ENABLE"} };
DD$FIELD_CODE  fc_130[] = { 
  {1,"INTGR OVRFLW TRP ENABL"} };
DD$FIELD_CODE  fc_131[] = { 
  {1,"FLTG UNDRFLW XCPTN ENBL"} };
DD$FIELD_CODE  fc_132[] = { 
  {1,"DEC OVRFLW TRP ENABLE"} };
DD$FIELD_CODE  fc_133[] = { 
  {0,"KERNEL"},
  {1,"EXEC"},
  {2,"SUPRVSR"},
  {3,"USER"} };
DD$FIELD_CODE  fc_134[] = { 
  {0,"KERNEL"},
  {1,"EXEC"},
  {2,"SUPRVSR"},
  {3,"USER"} };
DD$FIELD_CODE  fc_135[] = { 
  {1,"INTERRUPT STK"} };
DD$FIELD_CODE  fc_136[] = { 
  {1,"FIRST PART DONE"} };
DD$FIELD_CODE  fc_137[] = { 
  {1,"TRACE PENDING"} };
DD$FIELD_CODE  fc_138[] = { 
  {1,"PDP-11 CMPTBL MODE"},
  {0,"NATIVE MODE"} };
DD$FIELD_CODE  fc_139[] = { 
  {0,"CP READ TIMOUT/ERROR CONFIRMATION FAULT"},
  {2,"CP TRANSLATION BUFFER PE"},
  {3,"CP CACHE PE"},
  {5,"CP READ DATA SUBSTITUTE FAULT"},
  {10,"IB TRANSLATION BUFFER PE"},
  {12,"IB READ DATA SUBSTITUTE FAULT"},
  {13,"IB READ TIMEOUT/ERROR CONFIRMATION FAULT"},
  {15,"IB CACHE PE"},
  {241,"CONTROL STORE PE ABORT"},
  {242,"CP TRANSLATION BUFFER PE ABORT"},
  {243,"CP CACHE PE ABORT"},
  {244,"CP READ TIMEOUT/ERROR CONFIRMATION ABORT"},
  {245,"CP READ DATA SUBSTITUTE ABORT"},
  {246,"UCODE ABORT"} };
DD$FIELD_CODE  fc_140[] = { 
  {1,"PERFORMANCE MONITOR ENABLED"} };
DD$FIELD_CODE  fc_141[] = { 
  {1,"INTEGER OVERFLOW"},
  {2,"INTEGER DIVIDE BY 0"},
  {3,"FLOAT OVERFLOW"},
  {4,"FLOAT DIVIDE BY 0"},
  {5,"FLOAT UNDERFLOW"},
  {6,"DECIMAL OVERFLOW"},
  {7,"DECIMAL DIVIDE BY 0"} };
DD$FIELD_CODE  fc_142[] = { 
  {1,"ALU C31"} };
DD$FIELD_CODE  fc_143[] = { 
  {1,"ALU Z"} };
DD$FIELD_CODE  fc_144[] = { 
  {1,"ALU N"} };
DD$FIELD_CODE  fc_145[] = { 
  {1,"E ALU Z"} };
DD$FIELD_CODE  fc_146[] = { 
  {1,"E ALU N"} };
DD$FIELD_CODE  fc_147[] = { 
  {4,"GROUP 2"},
  {2,"GROUP 1"},
  {1,"GROUP 0"} };
DD$FIELD_CODE  fc_148[] = { 
  {1,"CONTROL STORE PE"} };
DD$FIELD_CODE  fc_149[] = { 
  {1,"NESTED ERR"} };
DD$FIELD_CODE  fc_150[] = { 
  {1,"ENABLE MEMORY MANAGEMENT"} };
DD$FIELD_CODE  fc_151[] = { 
  {2,"GROUP 0, BYTE 0"},
  {3,"GROUP 0, BYTE 1"},
  {4,"GROUP 0, BYTE 2"},
  {5,"GROUP 1, BYTE 0"},
  {6,"GROUP 1, BYTE 1"},
  {7,"GROUP 1, BYTE 2"},
  {8,"GROUP 0, ADDRESS BYTE 0"},
  {9,"GROUP 0, ADDRESS BYTE 1"},
  {10,"GROUP 0, ADDRESS BYTE 2"},
  {11,"GROUP 1, ADDRESS BYTE 0"},
  {12,"GROUP 1, ADDRESS BYTE 1"},
  {13,"GROUP 1, ADDRESS BYTE 2"} };
DD$FIELD_CODE  fc_152[] = { 
  {1,"TB HIT, GROUP 0"} };
DD$FIELD_CODE  fc_153[] = { 
  {1,"TB HIT, GROUP 1"} };
DD$FIELD_CODE  fc_154[] = { 
  {1,"IB AUTO RELOAD"} };
DD$FIELD_CODE  fc_155[] = { 
  {1,"IB WCHK ON IB REFERENCE"} };
DD$FIELD_CODE  fc_156[] = { 
  {1,"FORCE TB MISS, GROUP 0"} };
DD$FIELD_CODE  fc_157[] = { 
  {1,"FORCE TB MISS, GROUP 1"} };
DD$FIELD_CODE  fc_158[] = { 
  {1,"FORCE REPLACE, GROUP 0"} };
DD$FIELD_CODE  fc_159[] = { 
  {1,"FORCE REPLACE, GROUP 1"} };
DD$FIELD_CODE  fc_160[] = { 
  {1,"WRITE GROUP 0 & GROUP 1"} };
DD$FIELD_CODE  fc_161[] = { 
  {1,"AUTOMATIC LOAD"} };
DD$FIELD_CODE  fc_162[] = { 
  {1,"PROTECTION VIOLATION"} };
DD$FIELD_CODE  fc_163[] = { 
  {1,"TB PARITY ERR"} };
DD$FIELD_CODE  fc_164[] = { 
  {1,"TB MISS ON LOAD"} };
DD$FIELD_CODE  fc_165[] = { 
  {1,"BAD IPA"} };
DD$FIELD_CODE  fc_166[] = { 
  {1,"ONE"},
  {0,"TWO"} };
DD$FIELD_CODE  fc_167[] = { 
  {1,"CP TB PE"} };
DD$FIELD_CODE  fc_168[] = { 
  {1,"SBI NOT BUSY"} };
DD$FIELD_CODE  fc_169[] = { 
  {1,"MULTIPLE CP ERR"} };
DD$FIELD_CODE  fc_170[] = { 
  {1,"IB SBI ERROR CONFIRMATION"} };
DD$FIELD_CODE  fc_171[] = { 
  {4,"IB TIMEOUT, NO DEVICE RESPONSE"},
  {5,"IB TIMEOUT, DEVICE BUSY"},
  {6,"IB TIMEOUT, RD DATA WAIT"} };
DD$FIELD_CODE  fc_172[] = { 
  {1,"IB RDS"} };
DD$FIELD_CODE  fc_173[] = { 
  {1,"CP SBI ERR CONFIRMATION"} };
DD$FIELD_CODE  fc_174[] = { 
  {4,"CP TIMEOUT, NO DEVICE RESPONSE"},
  {5,"CP TIMEOUT, DEVICE BUSY"},
  {6,"CP TIMEOUT, RD DATA WAIT"} };
DD$FIELD_CODE  fc_175[] = { 
  {1,"RDS"} };
DD$FIELD_CODE  fc_176[] = { 
  {1,"CRD"} };
DD$FIELD_CODE  fc_177[] = { 
  {1,"RDS INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_178[] = { 
  {0,"KERNEL"},
  {1,"EXEC"},
  {2,"SUPERVISR"},
  {3,"USER"} };
DD$FIELD_CODE  fc_179[] = { 
  {1,"PHYSICAL MEMORY REFERENCE"},
  {0,"VIRTUAL MEMORY REFERENCE"} };
DD$FIELD_CODE  fc_180[] = { 
  {1,"READ REFERENCE"},
  {0,"MODIFY REFERENCE"} };
DD$FIELD_CODE  fc_181[] = { 
  {1,"FORCE MISS, GROUP 0"} };
DD$FIELD_CODE  fc_182[] = { 
  {1,"FORCE MISS, GROUP 1"} };
DD$FIELD_CODE  fc_183[] = { 
  {1,"REPLACE GROUP 1"},
  {0,"REPLACE GROUP 0"} };
DD$FIELD_CODE  fc_184[] = { 
  {1,"FORCE REPLACEMENT"},
  {0,"RANDOM REPLACEMENT"} };
DD$FIELD_CODE  fc_185[] = { 
  {1,"CACHE HIT"} };
DD$FIELD_CODE  fc_186[] = { 
  {1,"LOST ERROR"} };
DD$FIELD_CODE  fc_187[] = { 
  {1,"CACHE DATA PE"} };
DD$FIELD_CODE  fc_188[] = { 
  {1,"CACHE TAG PE"} };
DD$FIELD_CODE  fc_189[] = { 
  {1,"CORRECTED DATA ERR"} };
DD$FIELD_CODE  fc_190[] = { 
  {1,"LOST ERROR"} };
DD$FIELD_CODE  fc_191[] = { 
  {1,"UNCORRECTABLE ERR"} };
DD$FIELD_CODE  fc_192[] = { 
  {1,"MEMORY ERROR"} };
DD$FIELD_CODE  fc_193[] = { 
  {1,"FETCH FOR ISTRM"},
  {0,"OPERAND FETCH"} };
DD$FIELD_CODE  fc_194[] = { 
  {1,"TB PARITY ERR"} };
DD$FIELD_CODE  fc_195[] = { 
  {1,"BUS ERROR"} };
DD$FIELD_CODE  fc_196[] = { 
  {1,"MCK VIA INTERRUPT"} };
DD$FIELD_CODE  fc_197[] = { 
  {1,"MCK IN PROGRESS"} };
DD$FIELD_CODE  fc_198[] = { 
  {1,"ABORT"} };
DD$FIELD_CODE  fc_199[] = { 
  {1,"DECODER TO CONSOLE PE"} };
DD$FIELD_CODE  fc_200[] = { 
  {1,"CS2 PARITY ERR"} };
DD$FIELD_CODE  fc_201[] = { 
  {1,"CS1 PARITY ERR"} };
DD$FIELD_CODE  fc_202[] = { 
  {1,"CS0 PARITY ERR"} };
DD$FIELD_CODE  fc_203[] = { 
  {1,"CONSOLE TO DECODER PE"} };
DD$FIELD_CODE  fc_204[] = { 
  {1,"DECODER RAM OUTPUT PE"} };
DD$FIELD_CODE  fc_205[] = { 
  {1,"DECODER IPR PE"} };
DD$FIELD_CODE  fc_206[] = { 
  {1,"SEQ IPR PE"} };
DD$FIELD_CODE  fc_207[] = { 
  {1,"IB PE, LOWER WORD"} };
DD$FIELD_CODE  fc_208[] = { 
  {1,"IB PE, UPPER WORD"} };
DD$FIELD_CODE  fc_209[] = { 
  {1,"BAD UADDRESS"} };
DD$FIELD_CODE  fc_210[] = { 
  {1,"PIBA BROKEN"} };
DD$FIELD_CODE  fc_211[] = { 
  {1,"NMI DATA PE"} };
DD$FIELD_CODE  fc_212[] = { 
  {1,"BAD PIBA DATA"} };
DD$FIELD_CODE  fc_213[] = { 
  {1,"BAD READ DATA"} };
DD$FIELD_CODE  fc_214[] = { 
  {1,"NMI CS PE"} };
DD$FIELD_CODE  fc_215[] = { 
  {1,"TB TAG PE"} };
DD$FIELD_CODE  fc_216[] = { 
  {1,"MEMORY DATA PE"},
  {2,"MEMORY DATA PE"},
  {3,"MEMORY DATA PE"},
  {4,"MEMORY DATA PE"},
  {5,"MEMORY DATA PE"},
  {6,"MEMORY DATA PE"},
  {7,"MEMORY DATA PE"},
  {8,"MEMORY DATA PE"},
  {9,"MEMORY DATA PE"},
  {10,"MEMORY DATA PE"},
  {11,"MEMORY DATA PE"},
  {12,"MEMORY DATA PE"},
  {13,"MEMORY DATA PE"},
  {14,"MEMORY DATA PE"},
  {15,"MEMORY DATA PE"},
  {16,"MEMORY DATA PE"},
  {17,"MEMORY DATA PE"} };
DD$FIELD_CODE  fc_217[] = { 
  {1,"CACHE TAG PE"},
  {2,"CACHE TAG PE"},
  {3,"CACHE TAG PE"},
  {4,"CACHE TAG PE"},
  {5,"CACHE TAG PE"},
  {6,"CACHE TAG PE"},
  {7,"CACHE TAG PE"} };
DD$FIELD_CODE  fc_218[] = { 
  {1,"TB DATA PE"},
  {2,"TB DATA PE"},
  {3,"TB DATA PE"},
  {4,"TB DATA PE"},
  {5,"TB DATA PE"},
  {6,"TB DATA PE"},
  {7,"TB DATA PE"} };
DD$FIELD_CODE  fc_219[] = { 
  {1,"VA PE"},
  {2,"VA PE"},
  {3,"VA PE"} };
DD$FIELD_CODE  fc_220[] = { 
  {2,"INTERLOCK TIMEOUT"},
  {3,"NO RETURN RD DATA"},
  {4,"NO ACCESS, NO RESPONSE"},
  {5,"NO ACCESS TO BUS"},
  {6,"NO ACCESS, INTERLOCKED"},
  {7,"NO ACCESS, BUSY"} };
DD$FIELD_CODE  fc_221[] = { 
  {1,"TRANSMITTER DURING FLT"} };
DD$FIELD_CODE  fc_222[] = { 
  {1,"RD SEQUENCE ERR"} };
DD$FIELD_CODE  fc_223[] = { 
  {1,"CONTROL PE"} };
DD$FIELD_CODE  fc_224[] = { 
  {1,"ADDRESS/DATA PE"} };
DD$FIELD_CODE  fc_225[] = { 
  {1,"WRITE TIMEOUT"},
  {2,"READ TIMEOUT"},
  {3,"PIBA TIMEOUT"} };
DD$FIELD_CODE  fc_226[] = { 
  {1,"NMI FAULT"} };
DD$FIELD_CODE  fc_227[] = { 
  {1,"MEMORY ADDRESS REGISTER LOCK"} };
DD$FIELD_CODE  fc_228[] = { 
  {1,"PORT CONTROLLER DETECTED ERR"} };
DD$FIELD_CODE  fc_229[] = { 
  {1,"CACHE TAG PE"} };
DD$FIELD_CODE  fc_230[] = { 
  {1,"MTB MISS"} };
DD$FIELD_CODE  fc_231[] = { 
  {1,"BTB TAG PE"} };
DD$FIELD_CODE  fc_232[] = { 
  {1,"PORT CONTROLLER TIMEOUT"} };
DD$FIELD_CODE  fc_233[] = { 
  {1,"BTB OR CACHE DATA PE"} };
DD$FIELD_CODE  fc_234[] = { 
  {1,"MCP"},
  {2,"AKRSD"},
  {3,"BTO"},
  {4,"STP"},
  {5,"RCR"},
  {6,"IRW"},
  {7,"ARCR"},
  {8,"NICI"},
  {9,"NICIP"},
  {10,"AKRE"},
  {11,"IAL"},
  {12,"EVS4"},
  {13,"EVS5"} };
DD$FIELD_CODE  fc_235[] = { 
  {1,"WRITE MEMORY"} };
DD$FIELD_CODE  fc_236[] = { 
  {1,"VAXBI ERROR"} };
DD$FIELD_CODE  fc_237[] = { 
  {1,"VAX CAN'T RETRY"} };
DD$FIELD_CODE  fc_238[] = { 
  {1,"SBI NOT BUSY"} };
DD$FIELD_CODE  fc_239[] = { 
  {1,"MULTIPLE CP ERR"} };
DD$FIELD_CODE  fc_240[] = { 
  {1,"IB SBI ERR CONFIRMATION"} };
DD$FIELD_CODE  fc_241[] = { 
  {4,"NO DEVICE RESP"},
  {5,"DEVICE BUSY"},
  {6,"RD DATA WAIT"} };
DD$FIELD_CODE  fc_242[] = { 
  {1,"IB RDS"} };
DD$FIELD_CODE  fc_243[] = { 
  {1,"CP SBI ERR CONFIRMATION"} };
DD$FIELD_CODE  fc_244[] = { 
  {4,"NO DEVICE RESP"},
  {5,"DEVICE BUSY"},
  {6,"RD DATA WAIT"} };
DD$FIELD_CODE  fc_245[] = { 
  {1,"RDS"} };
DD$FIELD_CODE  fc_246[] = { 
  {1,"CRD"} };
DD$FIELD_CODE  fc_247[] = { 
  {1,"RDS/CRD INTERRUPT ENA"} };
DD$FIELD_CODE  fc_248[] = { 
  {1,"FAULT SILO LOCK"} };
DD$FIELD_CODE  fc_249[] = { 
  {1,"SBI FAULT"} };
DD$FIELD_CODE  fc_250[] = { 
  {1,"FAULT INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_251[] = { 
  {1,"FAULT LATCH"} };
DD$FIELD_CODE  fc_252[] = { 
  {1,"TRANSMITTER DURING FLT"} };
DD$FIELD_CODE  fc_253[] = { 
  {1,"MULTIPLE TRANSMITTER FLT"} };
DD$FIELD_CODE  fc_254[] = { 
  {1,"UNEXPECTD READ DATA"} };
DD$FIELD_CODE  fc_255[] = { 
  {1,"PARITY FLT"} };
DD$FIELD_CODE  fc_256[] = { 
  {1,"LOCK UNCONDITIONAL"} };
DD$FIELD_CODE  fc_257[] = { 
  {1,"SILO LOCKK INTERRUPT ENA"} };
DD$FIELD_CODE  fc_258[] = { 
  {1,"COMPARATOR SILO LOCK"} };
DD$FIELD_CODE  fc_259[] = { 
  {1,"FORCE TIMEOUT"} };
DD$FIELD_CODE  fc_260[] = { 
  {0,"NO TAG MATCH"},
  {1,"MATCH IN GROUP 1"},
  {2,"MATCH IN GROUP 0"} };
DD$FIELD_CODE  fc_261[] = { 
  {1,"FORCE P(1) REVERSAL ON SBI"} };
DD$FIELD_CODE  fc_262[] = { 
  {1,"DISABLE SBI CYCLES"} };
DD$FIELD_CODE  fc_263[] = { 
  {0,"FORCE REPLACEMENT ON GROUP 1 OR 0"},
  {1,"FORCE REPLACEMENT ON GROUP 1"},
  {2,"FORCE REPLACEMENT ONGROUP 0"} };
DD$FIELD_CODE  fc_264[] = { 
  {1,"FORCE MISS ON GROUP 1"},
  {2,"FORCE MISS ON GROUP 0"},
  {3,"FORCE MISS ON GROUP 1 & 0"} };
DD$FIELD_CODE  fc_265[] = { 
  {1,"ENABLE SBI INVALIDATE"} };
DD$FIELD_CODE  fc_266[] = { 
  {1,"FORCE SBI INVALIDATE"} };
DD$FIELD_CODE  fc_267[] = { 
  {1,"FORCE MULTIPLE TRANSMITTER FAULT"} };
DD$FIELD_CODE  fc_268[] = { 
  {1,"FORCE UNEXPECTD RD DATA FLT"} };
DD$FIELD_CODE  fc_269[] = { 
  {1,"FORCE WRT SEQUENCE FLT"} };
DD$FIELD_CODE  fc_270[] = { 
  {1,"FORCE P(0) REVERSAL ON SBI"} };
DD$FIELD_CODE  fc_271[] = { 
  {1,"ENABLE SHORT TIMEOUT"} };
DD$FIELD_CODE  fc_272[] = { 
  {1,"FORCE STATE PE"} };
DD$FIELD_CODE  fc_273[] = { 
  {1,"LOOP BACK MODE"} };
DD$FIELD_CODE  fc_274[] = { 
  {1,"FORCE QUADWORD DATA"} };
DD$FIELD_CODE  fc_275[] = { 
  {1,"DISABLE SBI TIMEOUT"} };
DD$FIELD_CODE  fc_276[] = { 
  {1,"DIAGNOSTIC DEAD"} };
DD$FIELD_CODE  fc_277[] = { 
  {1,"DISABLE SILO INCREMENT"} };
DD$FIELD_CODE  fc_278[] = { 
  {1,"CLEAR SILO ADDRESS"} };
DD$FIELD_CODE  fc_279[] = { 
  {1,"FORCE DMAI BUSY"} };
DD$FIELD_CODE  fc_280[] = { 
  {1,"FORCE DMAA BUSY"} };
DD$FIELD_CODE  fc_281[] = { 
  {1,"FORCE DMAB BUSY"} };
DD$FIELD_CODE  fc_282[] = { 
  {1,"FORCE DMAC BUSY"} };
DD$FIELD_CODE  fc_283[] = { 
  {1,"DMAI MBOX ERR"} };
DD$FIELD_CODE  fc_284[] = { 
  {1,"DMAI CONTROL ERR"} };
DD$FIELD_CODE  fc_285[] = { 
  {1,"DMAI A/D ERR"} };
DD$FIELD_CODE  fc_286[] = { 
  {1,"DMAI TIMEOUT"} };
DD$FIELD_CODE  fc_287[] = { 
  {1,"DMAA MBOX ERR"} };
DD$FIELD_CODE  fc_288[] = { 
  {1,"DMAA CONTROL ERR"} };
DD$FIELD_CODE  fc_289[] = { 
  {1,"DMAA A/D ERR"} };
DD$FIELD_CODE  fc_290[] = { 
  {1,"DMAB MBOX ERR"} };
DD$FIELD_CODE  fc_291[] = { 
  {1,"DMAB CNTRL ERR"} };
DD$FIELD_CODE  fc_292[] = { 
  {1,"DMAB A/D ERR"} };
DD$FIELD_CODE  fc_293[] = { 
  {1,"DMAC MBOX ERR"} };
DD$FIELD_CODE  fc_294[] = { 
  {1,"DMAC CONTROL ERR"} };
DD$FIELD_CODE  fc_295[] = { 
  {1,"DMAC A/D ERR"} };
DD$FIELD_CODE  fc_296[] = { 
  {1,"MULTIPLE CPU ERR"} };
DD$FIELD_CODE  fc_297[] = { 
  {1,"FORCE PARITY TRAP"} };
DD$FIELD_CODE  fc_298[] = { 
  {1,"ERROR ON C/A"} };
DD$FIELD_CODE  fc_299[] = { 
  {1,"LOCAL ADDRESS ERR"} };
DD$FIELD_CODE  fc_300[] = { 
  {1,"CONTROL PE"} };
DD$FIELD_CODE  fc_301[] = { 
  {1,"A/D PE"} };
DD$FIELD_CODE  fc_302[] = { 
  {1,"CPU ERROR LOCK"} };
DD$FIELD_CODE  fc_303[] = { 
  {1,"ENABLE SBI CYCLES OUT"} };
DD$FIELD_CODE  fc_304[] = { 
  {1,"ENABLE SBI CYCLES IN"} };
DD$FIELD_CODE  fc_305[] = { 
  {1,"MASTER INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_306[] = { 
  {1,"CPU SBI ERROR CONFIRMATION"} };
DD$FIELD_CODE  fc_307[] = { 
  {4,"NO RESPONSE"},
  {5,"DEVICE BUSY"},
  {6,"RD DATA WAIT"} };
DD$FIELD_CODE  fc_308[] = { 
  {1,"FAULT SILO LOCK"} };
DD$FIELD_CODE  fc_309[] = { 
  {1,"SBI FAULT"} };
DD$FIELD_CODE  fc_310[] = { 
  {1,"FAULT INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_311[] = { 
  {1,"FAULT LATCH"} };
DD$FIELD_CODE  fc_312[] = { 
  {1,"SBI P0 PE"} };
DD$FIELD_CODE  fc_313[] = { 
  {1,"SBI P1 PE"} };
DD$FIELD_CODE  fc_314[] = { 
  {1,"SBI TRANSMITTER DURING FLT"} };
DD$FIELD_CODE  fc_315[] = { 
  {1,"MLTPL XMITTR FLT"} };
DD$FIELD_CODE  fc_316[] = { 
  {1,"INTERLOCK SEQUENCE FLT"} };
DD$FIELD_CODE  fc_317[] = { 
  {1,"UNEXPECTD RD DATA"} };
DD$FIELD_CODE  fc_318[] = { 
  {1,"WRITE SEQUENCE FLT"} };
DD$FIELD_CODE  fc_319[] = { 
  {1,"SBI PARITY FAULT"} };
DD$FIELD_CODE  fc_320[] = { 
  {1,"LOCK UNCONDITIONAL"} };
DD$FIELD_CODE  fc_321[] = { 
  {1,"SILO LOCK INTERRUPT ENA"} };
DD$FIELD_CODE  fc_322[] = { 
  {1,"COMPARATOR SILO LOCK"} };
DD$FIELD_CODE  fc_323[] = { 
  {1,"USE MAINTENANCE ID"} };
DD$FIELD_CODE  fc_324[] = { 
  {1,"FORCE ISR DATA"} };
DD$FIELD_CODE  fc_325[] = { 
  {1,"FORCE MAINTENANCE TRANSFER"} };
DD$FIELD_CODE  fc_326[] = { 
  {1,"FORCE TR SEQUENCE"} };
DD$FIELD_CODE  fc_327[] = { 
  {1,"FORCE SBI INTERRUPT REQUEST"} };
DD$FIELD_CODE  fc_328[] = { 
  {1,"FORCE READ DATA TIMEOUT"} };
DD$FIELD_CODE  fc_329[] = { 
  {1,"FORCE P1 REVERSAL ON SBI"} };
DD$FIELD_CODE  fc_330[] = { 
  {1,"FORCE MULTIPLE TRANSMITTER FAULT"} };
DD$FIELD_CODE  fc_331[] = { 
  {1,"FORCE UNEXPECTED RD FAULT"} };
DD$FIELD_CODE  fc_332[] = { 
  {1,"FORCE WRITE SEQUENCE FAULT"} };
DD$FIELD_CODE  fc_333[] = { 
  {1,"FORCE P0 REVERSAL ON SBI"} };
DD$FIELD_CODE  fc_334[] = { 
  {1,"UNIBUS INIT CMPLETE"} };
DD$FIELD_CODE  fc_335[] = { 
  {1,"UNIBUS POWER DOWN"} };
DD$FIELD_CODE  fc_336[] = { 
  {1,"UNIBUS INIT ASSERTED"} };
DD$FIELD_CODE  fc_337[] = { 
  {1,"ADAPTER POWER UP"} };
DD$FIELD_CODE  fc_338[] = { 
  {1,"ADAPTER POWER DOWN"} };
DD$FIELD_CODE  fc_339[] = { 
  {1,"TRANSMIT FLT"} };
DD$FIELD_CODE  fc_340[] = { 
  {1,"MULTIPLE TRANSMITTER FLT"} };
DD$FIELD_CODE  fc_341[] = { 
  {1,"INTERLOCK SEQUENCE FAULT"} };
DD$FIELD_CODE  fc_342[] = { 
  {1,"UNEXPECTED RD DATA FAULT"} };
DD$FIELD_CODE  fc_343[] = { 
  {1,"WRT SEQUENCE FAULT"} };
DD$FIELD_CODE  fc_344[] = { 
  {1,"PARITY FAULT"} };
DD$FIELD_CODE  fc_345[] = { 
  {1,"ADAPTER INIT"} };
DD$FIELD_CODE  fc_346[] = { 
  {1,"UNIBUS POWER FAIL"} };
DD$FIELD_CODE  fc_347[] = { 
  {1,"CONFIG INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_348[] = { 
  {1,"SBI TO UNIBUS ERROR INTRRUPT ENABLE"} };
DD$FIELD_CODE  fc_349[] = { 
  {1,"UNIBUS TO SBI ERROR INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_350[] = { 
  {1,"BR INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_351[] = { 
  {1,"INTERRUPT FIELD SWITCH"} };
DD$FIELD_CODE  fc_352[] = { 
  {1,"UNIBUS SSYN TIMEOUT"} };
DD$FIELD_CODE  fc_353[] = { 
  {1,"UNIBUS SELECT TIMEOUT"} };
DD$FIELD_CODE  fc_354[] = { 
  {1,"LOST ERROR BIT"} };
DD$FIELD_CODE  fc_355[] = { 
  {1,"MAP REGISTER PARITY FAILURE"} };
DD$FIELD_CODE  fc_356[] = { 
  {1,"INVALID MAP REGISTER"} };
DD$FIELD_CODE  fc_357[] = { 
  {1,"DATA PATH PE"} };
DD$FIELD_CODE  fc_358[] = { 
  {1,"CMD TRANSMIT TIMEOUT"} };
DD$FIELD_CODE  fc_359[] = { 
  {1,"CMD TRANSMIT ERROR"} };
DD$FIELD_CODE  fc_360[] = { 
  {1,"CRD"} };
DD$FIELD_CODE  fc_361[] = { 
  {1,"RDS"} };
DD$FIELD_CODE  fc_362[] = { 
  {1,"RD DATA TIMEOUT"} };
DD$FIELD_CODE  fc_363[] = { 
  {1,"BRRVR 4 FULL"} };
DD$FIELD_CODE  fc_364[] = { 
  {1,"BRRVR 5 FULL"} };
DD$FIELD_CODE  fc_365[] = { 
  {1,"BRRVR 6 FULL"} };
DD$FIELD_CODE  fc_366[] = { 
  {1,"BRRVR 7 FULL"} };
DD$FIELD_CODE  fc_367[] = { 
  {1,"UNIBUS INIT COMPLETE"} };
DD$FIELD_CODE  fc_368[] = { 
  {1,"UNIBUS POWER DOWN"} };
DD$FIELD_CODE  fc_369[] = { 
  {1,"UNIBUS INIT ASSERTED"} };
DD$FIELD_CODE  fc_370[] = { 
  {1,"ADAPTER POWER UP"} };
DD$FIELD_CODE  fc_371[] = { 
  {1,"ADAPTER POWER DOWN"} };
DD$FIELD_CODE  fc_372[] = { 
  {1,"MICROSEQUENCER OK"} };
DD$FIELD_CODE  fc_373[] = { 
  {1,"DEFEAT DATA PATH PARITY"} };
DD$FIELD_CODE  fc_374[] = { 
  {1,"DEFEAT MAP PARITY"} };
DD$FIELD_CODE  fc_375[] = { 
  {1,"DISABLE INTERRUPT"} };
DD$FIELD_CODE  fc_376[] = { 
  {0,"NONINTERLEAVED LOWER CONTROLLER"},
  {2,"NONINTERLEAVED UPPER CONTROLLER"},
  {1,"EXTERNAL INTERLEAVE LOWER CONTROLLER"},
  {3,"EXTERNAL INTERLEAVE UPPER CONTROLLER"},
  {4,"INTERNAL 2-WAY"} };
DD$FIELD_CODE  fc_377[] = { 
  {1,"64K"},
  {2,"256K"},
  {0,"NO ARRAY BOARDS"},
  {3,"BOTH ARRAY TYPES"} };
DD$FIELD_CODE  fc_378[] = { 
  {1,"CONTROLLER 0 MISCONFIGURE"} };
DD$FIELD_CODE  fc_379[] = { 
  {1,"CONTROLLER 1 MISCONFIGURE"} };
DD$FIELD_CODE  fc_380[] = { 
  {1,"MISCONFIGURE"} };
DD$FIELD_CODE  fc_381[] = { 
  {1,"CONTROLLER 0 PE"} };
DD$FIELD_CODE  fc_382[] = { 
  {1,"CONTROLLER 1 PE"} };
DD$FIELD_CODE  fc_383[] = { 
  {1,"ERROR SUMMARY"} };
DD$FIELD_CODE  fc_384[] = { 
  {1,"POWER UP IN PROGRESS"} };
DD$FIELD_CODE  fc_385[] = { 
  {1,"POWER DOWN IN PROGRESS"} };
DD$FIELD_CODE  fc_386[] = { 
  {1,"TRANSMIT FAULT"} };
DD$FIELD_CODE  fc_387[] = { 
  {1,"SBI MULTIPLE TRANSMIT FAULT"} };
DD$FIELD_CODE  fc_388[] = { 
  {1,"SBI INTERLOCK SEQQUENCE FAULT"} };
DD$FIELD_CODE  fc_389[] = { 
  {1,"SBI WRT SEQUENCE FAULT"} };
DD$FIELD_CODE  fc_390[] = { 
  {1,"SBI PARITY FAULT"} };
DD$FIELD_CODE  fc_391[] = { 
  {1,"REFRESH LOCK"} };
DD$FIELD_CODE  fc_392[] = { 
  {1,"1"},
  {2,"2"},
  {3,"3"} };
DD$FIELD_CODE  fc_393[] = { 
  {1,"FORCE DBUS PE"} };
DD$FIELD_CODE  fc_394[] = { 
  {0,"INIT IN PROGRESS"},
  {1,"MEM HAS VALID DATA"},
  {3,"INIT COMPLETE"} };
DD$FIELD_CODE  fc_395[] = { 
  {1,"START ADDRESS WRT ENABLE"} };
DD$FIELD_CODE  fc_396[] = { 
  {1,"USEQ PE"} };
DD$FIELD_CODE  fc_397[] = { 
  {1,"CRD"} };
DD$FIELD_CODE  fc_398[] = { 
  {1,"RDS"} };
DD$FIELD_CODE  fc_399[] = { 
  {1,"ERROR LOG REQUEST"} };
DD$FIELD_CODE  fc_400[] = { 
  {1,"HI ERROR RATE"} };
DD$FIELD_CODE  fc_401[] = { 
  {1,"INHIBIT CRD ON SBI"} };
DD$FIELD_CODE  fc_402[] = { 
  {1,"FORCE MICROSEQUENCER PE"} };
DD$FIELD_CODE  fc_403[] = { 
  {1,"MICROSEQUENCER PE"} };
DD$FIELD_CODE  fc_404[] = { 
  {1,"CRD"} };
DD$FIELD_CODE  fc_405[] = { 
  {1,"RDS"} };
DD$FIELD_CODE  fc_406[] = { 
  {1,"ERROR LOG REQUESTED"} };
DD$FIELD_CODE  fc_407[] = { 
  {1,"HIGH ERROR RATE"} };
DD$FIELD_CODE  fc_408[] = { 
  {1,"INHIBIT CRD"} };
DD$FIELD_CODE  fc_409[] = { 
  {1,"FORCE MICROSEQUENCER PE"} };
DD$FIELD_CODE  fc_410[] = { 
  {0,"DUAL ROUND ROBIN"},
  {1,"FIXED-HI PRIORITY"},
  {2,"FIXED-LO PRIORITY"},
  {3,"DISABLE"} };
DD$FIELD_CODE  fc_411[] = { 
  {1,"SOFT ERROR INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_412[] = { 
  {1,"HARD ERROR INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_413[] = { 
  {1,"UNLOCK WRT PENDING"} };
DD$FIELD_CODE  fc_414[] = { 
  {1,"NODE RESET"} };
DD$FIELD_CODE  fc_415[] = { 
  {1,"SELF TEST PASSED"} };
DD$FIELD_CODE  fc_416[] = { 
  {1,"SELF TEST CONTINUING"} };
DD$FIELD_CODE  fc_417[] = { 
  {1,"INIT"} };
DD$FIELD_CODE  fc_418[] = { 
  {1,"SOFT ERROR SUMMARY"} };
DD$FIELD_CODE  fc_419[] = { 
  {1,"HARD ERROR SUMMARY"} };
DD$FIELD_CODE  fc_420[] = { 
  {1,"NULL BUS PE"} };
DD$FIELD_CODE  fc_421[] = { 
  {1,"CORRECTED RD DATA"} };
DD$FIELD_CODE  fc_422[] = { 
  {1,"ID PARITY ERR"} };
DD$FIELD_CODE  fc_423[] = { 
  {1,"USER PARITY ENABLE"} };
DD$FIELD_CODE  fc_424[] = { 
  {1,"ILLEGAL CONFIRMATIOM ERROR"} };
DD$FIELD_CODE  fc_425[] = { 
  {1,"NON-EXISTANT ADDRESS"} };
DD$FIELD_CODE  fc_426[] = { 
  {1,"BUS TIMEOUT"} };
DD$FIELD_CODE  fc_427[] = { 
  {1,"STALL TIMEOUT"} };
DD$FIELD_CODE  fc_428[] = { 
  {1,"RETRY TIMEOUT"} };
DD$FIELD_CODE  fc_429[] = { 
  {1,"READ DATA SUBSTITUTE"} };
DD$FIELD_CODE  fc_430[] = { 
  {1,"SLAVE PARITY ERR"} };
DD$FIELD_CODE  fc_431[] = { 
  {1,"CMD PARITY ERR"} };
DD$FIELD_CODE  fc_432[] = { 
  {1,"IDENT VECTOR ERR"} };
DD$FIELD_CODE  fc_433[] = { 
  {1,"TRANSMITTER DURING FAULT"} };
DD$FIELD_CODE  fc_434[] = { 
  {1,"INTERLOCK SEQUENCE ERROR"} };
DD$FIELD_CODE  fc_435[] = { 
  {1,"MASTER PARITY ERR"} };
DD$FIELD_CODE  fc_436[] = { 
  {1,"CONTROL TRANSMIT ERROR"} };
DD$FIELD_CODE  fc_437[] = { 
  {1,"MASTER TRANSMIT CHECK ERROR"} };
DD$FIELD_CODE  fc_438[] = { 
  {1,"NO ACK TO MULTIPLE RESPONSE CMD"} };
DD$FIELD_CODE  fc_439[] = { 
  {1,"FORCE INTERRUPT"} };
DD$FIELD_CODE  fc_440[] = { 
  {1,"INTERRUPT SENT"} };
DD$FIELD_CODE  fc_441[] = { 
  {1,"INTERRUPT COMPLETE"} };
DD$FIELD_CODE  fc_442[] = { 
  {1,"INTERRUPT ABORT"} };
DD$FIELD_CODE  fc_443[] = { 
  {1,"29116 RAM CODED"},
  {2,"BAD BUS/REG"},
  {3,"IRAM MARCH"},
  {4,"BDP BUS LATCH"},
  {5,"IRAM MASK CHIP SEL"},
  {6,"BDP STORED ADDR"},
  {7,"IRAM ADDR INCE"},
  {8,"XLATE BUFFER"},
  {9,"CONDITION CODE"},
  {10,"29116 INSTR"},
  {11,"START/END ADDR REG TST"},
  {12,"BI WMCI"},
  {13,"BI WMCI/READ"},
  {14,"UNIBUS DATO/DATI"},
  {15,"BI TO UNIBUS IRCI/UWMCI"},
  {16,"UNIBUS TO BI DATI/DATO"},
  {17,"BUA ERROR"},
  {18,"BI INTR/IDENT"} };
DD$FIELD_CODE  fc_444[] = { 
  {1,"REGISTER COMPARE ENABLE"} };
DD$FIELD_CODE  fc_445[] = { 
  {1,"BUA ERROR INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_446[] = { 
  {1,"BAD BDP"} };
DD$FIELD_CODE  fc_447[] = { 
  {1,"INVALID MAP REGISTER"} };
DD$FIELD_CODE  fc_448[] = { 
  {1,"UNIBUS INTERLOCK ERROR"} };
DD$FIELD_CODE  fc_449[] = { 
  {1,"UNIBUS SSYN TIMEOUT"} };
DD$FIELD_CODE  fc_450[] = { 
  {1,"BI FAILURE"} };
DD$FIELD_CODE  fc_451[] = { 
  {1,"ERROR"} };
DD$FIELD_CODE  fc_452[] = { 
  {0,"CONTROLLER ERROR"},
  {1,"MEMORY ERROR WITH ADDRESS"},
  {2,"DISK TRANSFER ERROR"},
  {3,"SDI ERROR"},
  {4,"SMALL DISK ERROR"},
  {9,"BAD BLOCK REPLACEMENT ATTEMPT"},
  {5,"TAPE TRANSFER ERROR"},
  {6,"STI COMMUNICATION OR COMMAND FAILURE"},
  {7,"STI DRIVE ERROR LOG"},
  {8,"STI FORMATTER ERROR LOG"} };
DD$FIELD_CODE  fc_453[] = { 
  {1,"SEQUENCE NUMBER RESET"} };
DD$FIELD_CODE  fc_454[] = { 
  {1,"ERROR DURING REPLACEMENT"} };
DD$FIELD_CODE  fc_455[] = { 
  {1,"BAD BLOCK REPLACEMENT REQUEST"} };
DD$FIELD_CODE  fc_456[] = { 
  {1,"OPERATION CONTINUING"} };
DD$FIELD_CODE  fc_457[] = { 
  {1,"OPERATION SUCCESSFUL"} };
DD$FIELD_CODE  fc_458[] = { 
  {0,"SUCCESS"},
  {1,"INVALID COMMAND"},
  {2,"COMMAND ABORTED"},
  {3,"UNIT OFFLINE"},
  {4,"UNIT AVAILABLE"},
  {5,"MEDIA FORMAT ERROR"},
  {6,"UNIT WRITE PROTECTED"},
  {7,"COMPARE ERROR"},
  {8,"DATA ERROR"},
  {9,"HOST BUFFER ACCESS ERROR"},
  {10,"CONTROLLER ERROR"},
  {11,"DRIVE ERROR"},
  {16,"RECORD DATA TRUNCATED"},
  {20,"BAD BLOCK REPLACEMENT COMPLETE"},
  {21,"INVALID PARAMETER"},
  {22,"ACCESS DENIED"},
  {31,"MESSAGE FROM INTERNAL DIAGNOSTIC"} };
DD$FIELD_CODE  fc_459[] = { 
  {0,"NORMAL"},
  {1,"INVALID MESSAGE LENGTH"},
  {3,"UNIT UNKNOWN OR ONLINE TO ANOTHER CONTROLLER"},
  {4,"AVAILABLE"},
  {8,"SECTOR WRITTEN WITH \"FORCE ERROR\" MODIFIER"},
  {9,"HOST BUFFER ACCESS ERR, CAUSE UNKNOWN"},
  {10,"COMMAND TIMEOUT, RETRY LIMIT EXCEEDED"},
  {20,"BAD BLOCK SUCCESSFULLY REPLACED"},
  {32,"SPIN DOWN IGNORED"},
  {35,"NO VOLUME MOUNTED OR DRIVE DISABLED VIA RUN/STOP SWITCH"},
  {41,"ODD TRANSFER ADDRESS"},
  {42,"SERDES OVERRUN OR UNDERRUN"},
  {43,"DRIVE COMMAND TIMED OUT"},
  {53,"BLOCK VERIFIED OK; NOT A BAD BLOCK"},
  {64,"STILL CONNECTED"},
  {67,"UNIT INOPERATIVE"},
  {72,"INVALID HEADER"},
  {73,"ODD BYTE COUNT"},
  {74,"EDC ERROR"},
  {75,"CONTROLLER DETECTED TRANSMISSION ERROR"},
  {84,"REPLACEMENT FAILED, REPLACE COMMAND FAILED"},
  {104,"DATA SYNC TIMEOUT"},
  {105,"NON-EXISTENT MEMORY"},
  {106,"INCONSISTENT INTERNAL CONTROL STRUCTURE"},
  {107,"POSITIONER ERROR"},
  {116,"REPLACEMENT FAILURE, INCONSISTENT RCT"},
  {128,"DUPLICATE UNIT NUMBER"},
  {131,"DUPLICATE UNIT NUMBER"},
  {136,"CORRECTABLE ERROR IN ECC FIELD"},
  {138,"INTERNAL EDC ERROR"},
  {139,"LOST READ/WRITE READY DURING/BETWEEN TRANSFERS"},
  {148,"REPLACEMENT FAILURE, DRIVE ACCESS FAILURE"},
  {165,"DISK NOT FORMATTED WITH 512 BYTE SECTORS"},
  {169,"INVALID PAGE TABLE ENTRY"},
  {170,"LESI ADAPTER CARD INPUT PARITY ERR"},
  {171,"DRIVE CLOCK DROPOUT"},
  {180,"NO REPLACEMENT BLOCK AVAILABLE"},
  {197,"DISK NOT FORMATTED OR FCT CORRUPTED"},
  {201,"INVALID BUFFER NAME"},
  {202,"LESI ADAPTER CARD OUTPUT PARITY ERR"},
  {203,"LOST RECEIVER READY FOR TRANSFER"},
  {212,"REPLACEMENT FAILURE, RECURSION FAILURE"},
  {229,"FCT UNREADABLE, UNCORRECTABLE ECC ERR"},
  {232,"UNCORRECTABLE ECC ERROR"},
  {233,"BUFFER LENGTH VIOLATION"},
  {234,"LESI ADAPTER CARD \"CABLE IN PLACE\" NOT ASSERTED"},
  {235,"DRIVE DETECTED ERROR"},
  {256,"ALREADY ONLINE"},
  {259,"UNIT DISABLED BY F/S OR DIAGNOSTIC"},
  {261,"RCT CORRUPTED"},
  {262,"UNIT IS DATA SAFETY WRITE PROTECTED"},
  {264,"ONE SYMBOL ECC ERROR"},
  {265,"ACCESS CONTROL VIOLATION"},
  {266,"CONTROLLER OVERRUN OR UNDERRUN"},
  {267,"CONTROLLER DETECTED PULSE/STATE PARITY ERROR"},
  {293,"NO REPLACEMENT BLOCK AVAILABLE"},
  {296,"TWO SYMBOL ECC ERROR"},
  {298,"CONTROLLER MEMORY ERROR"},
  {328,"THREE SYMBOL ECC ERROR"},
  {331,"CONTROLLER DETECTED PROTOCOL ERR"},
  {360,"FOUR SYMBOL ECC ERROR"},
  {363,"DRIVE FAILED INIT"},
  {392,"FIVE SYMBOL ECC ERROR"},
  {395,"DRIVE IGNORED INIT"},
  {424,"SIX SYMBOL ECC ERROR"},
  {427,"RECEIVER READY COLLISION"},
  {456,"SEVEN SYMBOL ECC ERROR"},
  {488,"EIGHT SYMBOL ECC ERROR"},
  {512,"STILL ONLINE"},
  {515,"EXCLUSIVE USE"},
  {520,"NINE SYMBOL ECC ERROR"},
  {552,"TEN SYMBOL ECC ERROR"},
  {584,"ELEVEN SYMBOL ECC ERROR"},
  {616,"TWELVE SYMBOL ECC ERROR"},
  {648,"THIRTEEN SYMBOL ECC ERROR"},
  {680,"FOURTEEN SYMBOL ECC ERROR"},
  {712,"FIFTEEN SYMBOL ECC ERROR"},
  {1024,"INCOMPLETE REPLACEMENT"},
  {1028,"ALREADY IN USE"},
  {2048,"INVALID RCT"},
  {4102,"UNIT IS SOFTWARE WRITE PROTECTED"},
  {4096,"READ ONLY VOLUME FORMAT"},
  {8198,"UNIT IS HARDWARE WRITE PROTECTED"} };
DD$FIELD_CODE  fc_460[] = { 
  {1,"HSC50"},
  {2,"UDA50"},
  {3,"RC25"},
  {4,"VMS"},
  {5,"TU81 INTEGRATED CONTROLLER"},
  {6,"UDA50A"},
  {7,"RQDX1/RQDX2"},
  {8,"TOPS 10/20"},
  {9,"TK50"},
  {10,"RUX50"},
  {12,"KFBTA"},
  {13,"KDA50-Q"},
  {14,"TK70"},
  {15,"RV80"},
  {16,"RRD50"},
  {18,"KDB50"},
  {19,"RQDX3"},
  {32,"HSC70"},
  {64,"KSB50"},
  {248,"ULTRIX-32"} };
DD$FIELD_CODE  fc_461[] = { 
  {1,"MASS STORAGE CONTROLLER"} };
DD$FIELD_CODE  fc_462[] = { 
  {1,"RA80"},
  {2,"RC25"},
  {3,"RCF25"},
  {4,"RA60"},
  {5,"RA81"},
  {6,"RD51"},
  {7,"RX50"},
  {8,"RD52"},
  {9,"RD53"},
  {10,"RX33"},
  {11,"RA82"},
  {12,"RD31"},
  {13,"RD54"},
  {14,"RRD50"},
  {17,"RX18"},
  {18,"RA70"},
  {19,"RA80"} };
DD$FIELD_CODE  fc_463[] = { 
  {2,"DEC STD 166 DISK"},
  {3,"TAPE CLASS DEVICE"},
  {4,"DEC STD 144 DISK DEVICE"} };
DD$FIELD_CODE  fc_464[] = { 
  {1,"BAD RBN"} };
DD$FIELD_CODE  fc_465[] = { 
  {1,"RCT INCONSISTENT"} };
DD$FIELD_CODE  fc_466[] = { 
  {1,"REFORMAT ERROR"} };
DD$FIELD_CODE  fc_467[] = { 
  {1,"TERTIARY REVECTOR"} };
DD$FIELD_CODE  fc_468[] = { 
  {1,"FORCE ERROR"} };
DD$FIELD_CODE  fc_469[] = { 
  {1,"REPLACEMENT ATTEMPTED"} };
DD$FIELD_CODE  fc_470[] = { 
  {1,"RUN/STOP SWITCH IN"},
  {0,"RUN/STOP SWITCH OUT"} };
DD$FIELD_CODE  fc_471[] = { 
  {1,"PORT SWITCH IN"},
  {0,"PORT SWITCH OUT"} };
DD$FIELD_CODE  fc_472[] = { 
  {1,"LOGGABLE INFO IN EXTNDED STATUS AREA"},
  {0,"NO LOGGABLE INFO IN EXTNDED STS AREA"} };
DD$FIELD_CODE  fc_473[] = { 
  {1,"SPINDLE READY"},
  {0,"SPINDLE NOT READY"} };
DD$FIELD_CODE  fc_474[] = { 
  {1,"DIAGNOSTIC LOAD REQUEST"} };
DD$FIELD_CODE  fc_475[] = { 
  {1,"READJUSTMENT REQUESTED"},
  {0,"NO READJUSTMENT REQUESTED"} };
DD$FIELD_CODE  fc_476[] = { 
  {1,"DRIVE ONLINE TO ANOTHER CONTROLLER"},
  {0,"DRIVE AVAILABLE TO CONTROLLER"} };
DD$FIELD_CODE  fc_477[] = { 
  {1,"576 BYTE SECTOR FORMAT"},
  {0,"512 BYTE SECTOR FORMAT"} };
DD$FIELD_CODE  fc_478[] = { 
  {1,"DIAGNOSTIC CYLINDER ACCESS ENABLED"} };
DD$FIELD_CODE  fc_479[] = { 
  {1,"FORMATTING OPERATION ENABLED"},
  {0,"FORMATTING OPERATION DISABLED"} };
DD$FIELD_CODE  fc_480[] = { 
  {1,"DRIVE DISABLED BY DIAGNOSTIC OR ERROR ROUTINE"},
  {0,"DRIVE ENABLED BY DIAGNOSTIC OR ERROR ROUTINE"} };
DD$FIELD_CODE  fc_481[] = { 
  {1,"WRITE PROTECT SWITCH FOR UNIT 0 IN"},
  {0,"WRITE PROTECT SWITCH FOR UNIT 0 OUT"} };
DD$FIELD_CODE  fc_482[] = { 
  {1,"WRITE LOCK ERROR"} };
DD$FIELD_CODE  fc_483[] = { 
  {1,"DRIVE FAILURE DURING INIT"} };
DD$FIELD_CODE  fc_484[] = { 
  {1,"LEVEL 2 PROTOCOL ERROR"} };
DD$FIELD_CODE  fc_485[] = { 
  {1,"SDI TRANSMISSION ERR"} };
DD$FIELD_CODE  fc_486[] = { 
  {1,"DRIVE ERROR"} };
DD$FIELD_CODE  fc_487[] = { 
  {0,"NORMAL DRIVE OPERATION"},
  {8,"DRIVE BEING USED BY DIAGNOSTIC"},
  {9,"UNIT SELECTION ID SAME AS ANOTHER DRIVE"} };
DD$FIELD_CODE  fc_488[] = { 
  {1,"RUN/STOP SWITCH IN"},
  {0,"RUN/STOP SWITCH OUT"} };
DD$FIELD_CODE  fc_489[] = { 
  {1,"PORT SWITCH IN"},
  {0,"PORT SWITCH OUT"} };
DD$FIELD_CODE  fc_490[] = { 
  {1,"PORT B RECEIVE ENABLED"},
  {0,"PORT A RECEIVE ENABLED"} };
DD$FIELD_CODE  fc_491[] = { 
  {1,"LOGGABLE INFO IN EXTENDED STATUS AREA"},
  {0,"NO LOGGABLE INFO IN EXTENDED STATUS AREA"} };
DD$FIELD_CODE  fc_492[] = { 
  {1,"SPINDLE READY"},
  {0,"SPINDLE NOT READY"} };
DD$FIELD_CODE  fc_493[] = { 
  {1,"DIAGNOSTIC LOAD REQUEST"} };
DD$FIELD_CODE  fc_494[] = { 
  {1,"INTERNAL READJUSTMENT REQUESTED"} };
DD$FIELD_CODE  fc_495[] = { 
  {1,"DRIVE ONLINE TO ANOTHER CONTROLLER"},
  {0,"DRIVE AVAILABLE TO CONTROLLER"} };
DD$FIELD_CODE  fc_496[] = { 
  {1,"576 BYTE SECTOR FORMAT"},
  {0,"512 BYTE SECTOR FORMAT"} };
DD$FIELD_CODE  fc_497[] = { 
  {1,"DIAGNOSTIC CYLINDER ACCESS ENABLED"},
  {0,"DIAGNOSTIC CYLINDER ACCESS DISABLED"} };
DD$FIELD_CODE  fc_498[] = { 
  {1,"FORMATTING OPERATIONS ENABLED"},
  {0,"FORMATTING OPERATIONS DISABLED"} };
DD$FIELD_CODE  fc_499[] = { 
  {1,"DRIVE DISABLED BY DIAGNOSTIC OR ERROR ROUTINE"},
  {0,"DRIVE ENABLED BY DIAGNOSTIC OR ERROR ROUTINE"} };
DD$FIELD_CODE  fc_500[] = { 
  {1,"UNIT 0 WRITE PROTECT SWITCH IN"},
  {0,"UNIT 0 WRITE PROTECT SWITCH OUT"} };
DD$FIELD_CODE  fc_501[] = { 
  {1,"WRITE LOCK ERROR"} };
DD$FIELD_CODE  fc_502[] = { 
  {1,"DRIVE FAILURE DURING INIT"} };
DD$FIELD_CODE  fc_503[] = { 
  {1,"LEVEL 2 PROTOCOL ERRROR"} };
DD$FIELD_CODE  fc_504[] = { 
  {1,"SDI TRANSMISSION ERROR"} };
DD$FIELD_CODE  fc_505[] = { 
  {1,"DRIVE ERROR"} };
DD$FIELD_CODE  fc_506[] = { 
  {8,"DRIVE BEING USED BY DIAGNOSTIC"},
  {9,"DRIVE OFFLINE, DUPLICATE UNIT ID"} };
DD$FIELD_CODE  fc_507[] = { 
  {3,"DIAGNOSE"},
  {5,"DRIVE CLEAR"},
  {6,"ERROR RECOVERY"},
  {10,"INITIATE SEEK"},
  {12,"RUN"},
  {15,"WRITE MEMORY"},
  {129,"CHANGE MODE"},
  {130,"CHANGE CONTROLLER FLAGS"},
  {132,"DISCONNECT"},
  {135,"GET COMMON CHARACTERISTICS"},
  {136,"GET SUB-UNIT CHARACTERISTICS"},
  {139,"ON-LINE"},
  {141,"READ MEMORY"},
  {142,"RECALIBRATE"},
  {144,"TOPOLOGY"},
  {255,"SELECT GROUP"} };
DD$FIELD_CODE  fc_508[] = { 
  {1,"READ/WRITE OVERRUN"} };
DD$FIELD_CODE  fc_509[] = { 
  {1,"PARITY ERR ON RCTS LINE"} };
DD$FIELD_CODE  fc_510[] = { 
  {1,"CONTROL PULSE ERROR"} };
DD$FIELD_CODE  fc_511[] = { 
  {1,"DATA PULSE ERROR"} };
DD$FIELD_CODE  fc_512[] = { 
  {0,"SUCCESS"},
  {1,"INVALID COMMAND"},
  {2,"COMMAND ABORTED"},
  {3,"UNIT OFFLINE"},
  {4,"UNIT AVAILABLE"},
  {6,"WRITE PROTECTION"},
  {7,"COMPARE ERROR"},
  {8,"DATA ERROR"},
  {9,"HOST BUFFER ACCESS ERROR"},
  {10,"CONTROLLER ERROR"},
  {11,"DRIVE ERROR"},
  {12,"FORMATTER ERROR"},
  {13,"BOT ENCOUNTERED"},
  {14,"TAPE MARK ENCOUNTERED"},
  {16,"RECORD DATA TRUNCATED"},
  {17,"POSITION LOST"},
  {18,"SERIOUS EXCEPTION"},
  {19,"LEOT DETECTED"},
  {31,"MESSAGE FRM INTRNL DIAGN"} };
DD$FIELD_CODE  fc_513[] = { 
  {0,"NORMAL"},
  {1,"INVALID MESSAGE LENGTH"},
  {3,"UNIT UNKNOWN OR ONLINE  TO ANOTHER CONTROLLER"},
  {8,"LONG GAP ENCOUNTERED"},
  {10,"CMD TIMEOUT, RETRY LIMIT EXCEEDED"},
  {35,"NO MEDIA MOUNTED OR DISABLED VIA SWITCH"},
  {64,"STILL CONNECTED"},
  {67,"UNIT INOPERATIVE"},
  {128,"UNIT SOFTWARE WRITE PROTECTED"},
  {131,"DUPLICATE UNIT NUMBER"},
  {232,"UNCORRECTABLE READ ERROR"},
  {256,"ALREADY ONLINE"},
  {259,"UNIT DISABLED BY F/S OR DIAGNOSTIC"},
  {512,"STILL ONLINE"},
  {520,"STILL ONLINE/UNLOAD IGNORED"},
  {1024,"EOT ENCOUNTERED"},
  {2048,"INVALID RCT"},
  {4102,"SOFTWARE WRITE PROTECTED"},
  {8198,"HARDWARE WRITE PROTECTED"} };
DD$FIELD_CODE  fc_514[] = { 
  {1,"TA78"},
  {3,"TK50"},
  {4,"TU81"} };
DD$FIELD_CODE  fc_515[] = { 
  {3,"TAPE CLASS DEVICE"},
  {2,"DEC STD 166 DISK DEVICE"},
  {4,"DEC STD 144 DISK DEVICE"} };
DD$FIELD_CODE  fc_516[] = { 
  {1,"DIAGNOSTIC REQUESTED"} };
DD$FIELD_CODE  fc_517[] = { 
  {1,"PORT SWITCH ENABLED"} };
DD$FIELD_CODE  fc_518[] = { 
  {1,"FORMATTER UNAVAILABLE"},
  {0,"FORMATTER AVAILABLE"} };
DD$FIELD_CODE  fc_519[] = { 
  {1,"DRIVE 0 ATTENTION"} };
DD$FIELD_CODE  fc_520[] = { 
  {1,"DRIVE 1 ATTENTION"} };
DD$FIELD_CODE  fc_521[] = { 
  {1,"DRIVE 2 ATTENTION"} };
DD$FIELD_CODE  fc_522[] = { 
  {1,"DRIVE 3 ATTENTION"} };
DD$FIELD_CODE  fc_523[] = { 
  {1,"FORMATTER ATTENTION"} };
DD$FIELD_CODE  fc_524[] = { 
  {1,"FORMATTER DIAGNOSTIC FAILED"} };
DD$FIELD_CODE  fc_525[] = { 
  {1,"LEVEL 2 PROTOCOL ERROR"} };
DD$FIELD_CODE  fc_526[] = { 
  {1,"LEVEL 1 TRANSMISSION ERROR"} };
DD$FIELD_CODE  fc_527[] = { 
  {1,"FORMATTER ERROR"} };
DD$FIELD_CODE  fc_528[] = { 
  {1,"RETRY: FAILURE DIR"} };
DD$FIELD_CODE  fc_529[] = { 
  {1,"RETRY: REQUEST TRANSFER"} };
DD$FIELD_CODE  fc_530[] = { 
  {1,"RETRY: REQUEST POSITION"} };
DD$FIELD_CODE  fc_531[] = { 
  {1,"ERROR LOG INFO AVAILABLE"} };
DD$FIELD_CODE  fc_532[] = { 
  {1,"ERROR LOG INFO AVAILABLE"} };
DD$FIELD_CODE  fc_533[] = { 
  {1,"MAINTENANCE MODE REQUEST"} };
DD$FIELD_CODE  fc_534[] = { 
  {1,"DRIVE AVAILABLE TO FORMATTER"} };
DD$FIELD_CODE  fc_535[] = { 
  {1,"DRIVE ONLINE TO FORMATTER"} };
DD$FIELD_CODE  fc_536[] = { 
  {1,"DRIVE WRITE LOCKED"} };
DD$FIELD_CODE  fc_537[] = { 
  {1,"BOT"} };
DD$FIELD_CODE  fc_538[] = { 
  {1,"EOT"} };
DD$FIELD_CODE  fc_539[] = { 
  {1,"TAPE MARK"} };
DD$FIELD_CODE  fc_540[] = { 
  {1,"DATA TRANSFER ERROR"} };
DD$FIELD_CODE  fc_541[] = { 
  {1,"EXCEPTION DURING DATA TRANSFER"} };
DD$FIELD_CODE  fc_542[] = { 
  {1,"POSITION LOST"} };
DD$FIELD_CODE  fc_543[] = { 
  {1,"LENGTHY OPERATION IN PROGRESS"} };
DD$FIELD_CODE  fc_544[] = { 
  {1,"DRIVE ERROR"} };
DD$FIELD_CODE  fc_545[] = { 
  {1,"ERROR LOG INFORMATION AVAILABLE"} };
DD$FIELD_CODE  fc_546[] = { 
  {1,"MAINTENANCE MODE REQUEST"} };
DD$FIELD_CODE  fc_547[] = { 
  {1,"DRIVE AVAILABLE TO FORMATTER"} };
DD$FIELD_CODE  fc_548[] = { 
  {1,"DRIVE ONLINE TO FORMATTER"} };
DD$FIELD_CODE  fc_549[] = { 
  {1,"DRIVE WRITE LOCKED"} };
DD$FIELD_CODE  fc_550[] = { 
  {1,"BOT"} };
DD$FIELD_CODE  fc_551[] = { 
  {1,"EOT"} };
DD$FIELD_CODE  fc_552[] = { 
  {1,"TAPE MARK ENCOUNTERED"} };
DD$FIELD_CODE  fc_553[] = { 
  {1,"DATA TRANSFER ERROR"} };
DD$FIELD_CODE  fc_554[] = { 
  {1,"EXCEPTION DURING DATA TRANSFER"} };
DD$FIELD_CODE  fc_555[] = { 
  {1,"POSITION LOST"} };
DD$FIELD_CODE  fc_556[] = { 
  {1,"LENGTHY OPERATION IN PROGRESS"} };
DD$FIELD_CODE  fc_557[] = { 
  {1,"DRIVE ERROR"} };
DD$FIELD_CODE  fc_558[] = { 
  {1,"ERROR LOG INFORMATION AVAILABLE"} };
DD$FIELD_CODE  fc_559[] = { 
  {1,"MAINTENANCE MODE REQUEST"} };
DD$FIELD_CODE  fc_560[] = { 
  {1,"DRIVE AVAILABLE TO FORMATTER"} };
DD$FIELD_CODE  fc_561[] = { 
  {1,"DRIVE ONLINE TO FORMATTER"} };
DD$FIELD_CODE  fc_562[] = { 
  {1,"DRIVE WRITE LOCKED"} };
DD$FIELD_CODE  fc_563[] = { 
  {1,"BOT"} };
DD$FIELD_CODE  fc_564[] = { 
  {1,"EOT"} };
DD$FIELD_CODE  fc_565[] = { 
  {1,"TAPE MARK ENCOUNTERED"} };
DD$FIELD_CODE  fc_566[] = { 
  {1,"DATA TRANSFER ERROR"} };
DD$FIELD_CODE  fc_567[] = { 
  {1,"EXCEPTION DURING DATA TRANSFER"} };
DD$FIELD_CODE  fc_568[] = { 
  {1,"POSITION LOST"} };
DD$FIELD_CODE  fc_569[] = { 
  {1,"LENGTHY OPERATION IN PROGRESS"} };
DD$FIELD_CODE  fc_570[] = { 
  {1,"DRIVE ERROR"} };
DD$FIELD_CODE  fc_571[] = { 
  {1,"ERROR LOG INFORMATION AVAILABLE"} };
DD$FIELD_CODE  fc_572[] = { 
  {1,"MAINTENANCE MODE REQUEST"} };
DD$FIELD_CODE  fc_573[] = { 
  {1,"DRIVE AVAILABLE TO FORMATTER"} };
DD$FIELD_CODE  fc_574[] = { 
  {1,"DRIVE ONLINE TO FORMATTER"} };
DD$FIELD_CODE  fc_575[] = { 
  {1,"DRIVE WRITE LOCKED"} };
DD$FIELD_CODE  fc_576[] = { 
  {1,"BOT"} };
DD$FIELD_CODE  fc_577[] = { 
  {1,"EOT"} };
DD$FIELD_CODE  fc_578[] = { 
  {1,"TAPE MARK ENCOUNTERED"} };
DD$FIELD_CODE  fc_579[] = { 
  {1,"DATA TRANSFER ERROR"} };
DD$FIELD_CODE  fc_580[] = { 
  {1,"EXCEPTION DURING DATA TRANSFER"} };
DD$FIELD_CODE  fc_581[] = { 
  {1,"POSITION LOST"} };
DD$FIELD_CODE  fc_582[] = { 
  {1,"LENGTHY OPERATION IN PROGRESS"} };
DD$FIELD_CODE  fc_583[] = { 
  {1,"DRIVE ERROR"} };
DD$FIELD_CODE  fc_584[] = { 
  {1,"BAD IPL"} };
DD$FIELD_CODE  fc_585[] = { 
  {1,"UCODE ERROR"} };
DD$FIELD_CODE  fc_586[] = { 
  {1,"MIB BUS PE"} };
DD$FIELD_CODE  fc_587[] = { 
  {1,"CACHE OR BTB PE"} };
DD$FIELD_CODE  fc_588[] = { 
  {1,"VAXBI MODE ERR"} };
DD$FIELD_CODE  fc_589[] = { 
  {1,"BTB TAG PE"} };
DD$FIELD_CODE  fc_590[] = { 
  {1,"CACHE TAG PE"} };
DD$FIELD_CODE  fc_591[] = { 
  {1,"PREFETCH ERR"} };
DD$FIELD_CODE  fc_592[] = { 
  {0,"NON-INTERLEAVED"},
  {1,"2-WAY"} };
DD$FIELD_CODE  fc_593[] = { 
  {0,"ERROR, NO ARRAY BOARDS"},
  {1,"4K MOS"},
  {2,"16K MOS"},
  {3,"ERROR, BOTH ARRAY BOARDS"} };
DD$FIELD_CODE  fc_594[] = { 
  {1,"INTRLEAVE ENABLE"} };
DD$FIELD_CODE  fc_595[] = { 
  {0,"64K"},
  {1,"128K"},
  {2,"192K"},
  {3,"256K"},
  {4,"320K"},
  {5,"384K"},
  {6,"448K"},
  {7,"512K"},
  {8,"576K"},
  {9,"640K"},
  {10,"704K"},
  {11,"768K"},
  {12,"832K"},
  {13,"896K"},
  {14,"960K"},
  {15,"1024K"} };
DD$FIELD_CODE  fc_596[] = { 
  {1,"POWER DOWN"} };
DD$FIELD_CODE  fc_597[] = { 
  {1,"POWER UP"} };
DD$FIELD_CODE  fc_598[] = { 
  {1,"SBI TRANSMIT FLT"} };
DD$FIELD_CODE  fc_599[] = { 
  {1,"MLTLP SBI TRANSMIT"} };
DD$FIELD_CODE  fc_600[] = { 
  {1,"INTERLOCK SEQUENCE FAULT"} };
DD$FIELD_CODE  fc_601[] = { 
  {1,"WRITE SEQUENCE ERROR"} };
DD$FIELD_CODE  fc_602[] = { 
  {1,"SBI PE"} };
DD$FIELD_CODE  fc_603[] = { 
  {1,"BYPASS ECC"} };
DD$FIELD_CODE  fc_604[] = { 
  {1,"FORCE ERR"} };
DD$FIELD_CODE  fc_605[] = { 
  {0,"INIT IN PROGRESS"},
  {1,"MEMORY HAS VALID DATA"},
  {3,"INIT COMPLETE"} };
DD$FIELD_CODE  fc_606[] = { 
  {1,"START ADDRSS WRITE ENABLE"} };
DD$FIELD_CODE  fc_607[] = { 
  {1,"ERROR LOG REQUEST"} };
DD$FIELD_CODE  fc_608[] = { 
  {1,"HIGH ERROR RATE"} };
DD$FIELD_CODE  fc_609[] = { 
  {1,"INHIBIT CRD"} };
DD$FIELD_CODE  fc_610[] = { 
  {1,"CRD ERROR LOG REQUESTED"} };
DD$FIELD_CODE  fc_611[] = { 
  {1,"RDS HIGH ERROR RATE"} };
DD$FIELD_CODE  fc_612[] = { 
  {1,"RDS ERROR LOG REQUESTED"} };
DD$FIELD_CODE  fc_613[] = { 
  {1,"DISABLE ECC FOR DATA"} };
DD$FIELD_CODE  fc_614[] = { 
  {1,"CHECK DIAGNOSTICS"} };
DD$FIELD_CODE  fc_615[] = { 
  {1,"PAGE MODE"} };
DD$FIELD_CODE  fc_616[] = { 
  {1,"ENABLE CRD ERR"} };
DD$FIELD_CODE  fc_617[] = { 
  {1,"POWER UP"} };
DD$FIELD_CODE  fc_618[] = { 
  {0,"16K RAM"} };
DD$FIELD_CODE  fc_619[] = { 
  {2,"INTERLOCK TIMEOUT"} };
DD$FIELD_CODE  fc_620[] = { 
  {1,"TRANSMIT DURING FAULT"} };
DD$FIELD_CODE  fc_621[] = { 
  {1,"WRITE SEQUENCE FAULT"} };
DD$FIELD_CODE  fc_622[] = { 
  {1,"READ SEQUENCE FAULT"} };
DD$FIELD_CODE  fc_623[] = { 
  {1,"CONTROL PARITY FAULT"} };
DD$FIELD_CODE  fc_624[] = { 
  {1,"DATA PARITY FAULT"} };
DD$FIELD_CODE  fc_625[] = { 
  {1,"MEMORY ARRAY BOARD PRESENT"} };
DD$FIELD_CODE  fc_626[] = { 
  {1,"DECODE RAM PARITY"} };
DD$FIELD_CODE  fc_627[] = { 
  {1,"ENABLE RAM DECODE"} };
DD$FIELD_CODE  fc_628[] = { 
  {1,"SUBSTITUTE CHECK BITS"} };
DD$FIELD_CODE  fc_629[] = { 
  {1,"FORCE BAD DATA"} };
DD$FIELD_CODE  fc_630[] = { 
  {1,"ENABLE LOOPBACK"} };
DD$FIELD_CODE  fc_631[] = { 
  {2,"FORCE COMMAND/ADDRESS PE"},
  {3,"FORCE WRITE DATA PE"} };
DD$FIELD_CODE  fc_632[] = { 
  {1,"ENABLE CHECK BIT SUBSTITUTION"} };
DD$FIELD_CODE  fc_633[] = { 
  {1,"DISABLE ECC CHECKING"} };
DD$FIELD_CODE  fc_634[] = { 
  {1,"ENABLE DIAGNOSTICS"} };
DD$FIELD_CODE  fc_635[] = { 
  {1,"BAD DATA"} };
DD$FIELD_CODE  fc_636[] = { 
  {1,"CRD ERROR LOG REQUEST"} };
DD$FIELD_CODE  fc_637[] = { 
  {1,"RDS ERROR LOG REQUEST"} };
DD$FIELD_CODE  fc_638[] = { 
  {1,"RDS HIGH ERROR RATE"} };
DD$FIELD_CODE  fc_639[] = { 
  {0,"NO BOARD"},
  {3,"4 MEG"},
  {4,"8 MEG"},
  {5,"16 MEG"},
  {6,"32 MEG"},
  {7,"CAPACITOR BOARD"} };
DD$FIELD_CODE  fc_640[] = { 
  {0,"NO BOARD"},
  {3,"4 MEG"},
  {4,"8 MEG"},
  {5,"16 MEG"},
  {6,"32 MEG"},
  {7,"CAPACITOR BOARD"} };
DD$FIELD_CODE  fc_641[] = { 
  {0,"NO BOARD"},
  {3,"4 MEG"},
  {4,"8 MEG"},
  {5,"16 MEG"},
  {6,"32 MEG"},
  {7,"CAPACITOR BOARD"} };
DD$FIELD_CODE  fc_642[] = { 
  {0,"NO BOARD"},
  {3,"4 MEG"},
  {4,"8 MEG"},
  {5,"16 MEG"},
  {6,"32 MEG"},
  {7,"CAPACITOR BOARD"} };
DD$FIELD_CODE  fc_643[] = { 
  {0,"NO BOARD"},
  {3,"4 MEG"},
  {4,"8 MEG"},
  {5,"16 MEG"},
  {6,"32 MEG"},
  {7,"CAPACITOR BOARD"} };
DD$FIELD_CODE  fc_644[] = { 
  {0,"NO BOARD"},
  {3,"4 MEG"},
  {4,"8 MEG"},
  {5,"16 MEG"},
  {6,"32 MEG"},
  {7,"CAPACITOR BOARD"} };
DD$FIELD_CODE  fc_645[] = { 
  {0,"NO BOARD"},
  {3,"4 MEG"},
  {4,"8 MEG"},
  {5,"16 MEG"},
  {6,"32 MEG"},
  {7,"CAPACITOR BOARD"} };
DD$FIELD_CODE  fc_646[] = { 
  {1,"NO BOARD"},
  {3,"4 MEG"},
  {4,"8 MEG"},
  {5,"16 MEG"},
  {6,"32 MEG"},
  {7,"CAPACITOR BOARD"} };
DD$FIELD_CODE  fc_647[] = { 
  {1,"INTERNAL ERROR"} };
DD$FIELD_CODE  fc_648[] = { 
  {1,"BATTERY BACKUP FAILED"} };
DD$FIELD_CODE  fc_649[] = { 
  {1,"ENABLE INTERRUPT"} };
DD$FIELD_CODE  fc_650[] = { 
  {1,"ENABLE DOUBLE BIT INTERRUPT"} };
DD$FIELD_CODE  fc_651[] = { 
  {1,"ENABLE SINGLE BIT INTERRUPT"} };
DD$FIELD_CODE  fc_652[] = { 
  {1,"ENABLE INTERLOCK TIMEOUT INTERRUPT"} };
DD$FIELD_CODE  fc_653[] = { 
  {1,"ENABLE INTERNAL ERROR INTERRUPT"} };
DD$FIELD_CODE  fc_654[] = { 
  {1,"INTERNAL INTERLEAVE"} };
DD$FIELD_CODE  fc_655[] = { 
  {1,"INTERNAL CONTROLLER ERROR"} };
DD$FIELD_CODE  fc_656[] = { 
  {1,"MASKED WRITE ERROR"} };
DD$FIELD_CODE  fc_657[] = { 
  {1,"BROKE"} };
DD$FIELD_CODE  fc_658[] = { 
  {1,"INTERLOCK ENABLR"} };
DD$FIELD_CODE  fc_659[] = { 
  {1,"MEMORY VALID"} };
DD$FIELD_CODE  fc_660[] = { 
  {1,"INHIBIT CRD"} };
DD$FIELD_CODE  fc_661[] = { 
  {1,"DISABLE ECC"} };
DD$FIELD_CODE  fc_662[] = { 
  {1,"ECC DIAGNOSTIC"} };
DD$FIELD_CODE  fc_663[] = { 
  {1,"ERROR"} };
DD$FIELD_CODE  fc_664[] = { 
  {1,"INTERNAL ADDRESS ERROR"} };
DD$FIELD_CODE  fc_665[] = { 
  {1,"CRD ERROR LOG REQUEST"} };
DD$FIELD_CODE  fc_666[] = { 
  {1,"HIGH ERROR RATE"} };
DD$FIELD_CODE  fc_667[] = { 
  {1,"RDS ERROR LOG REQUEST"} };
DD$FIELD_CODE  fc_668[] = { 
  {1,"29116 CONDITION CODE"},
  {2,"29116 INTERNAL RAM"},
  {3,"MAP RAM DATA & ADDRESS INTEGRITY"},
  {4,"BCAI OCTAWORD BUFFER"},
  {5,"BCAI OCTAWORD BUFFER WRAP"},
  {6,"WRITE MASK (MASK:1111,LONGWORD:0)"},
  {7,"WRITE MASK (MASK:0011,LONGWORD:1)"},
  {8,"WRITE MASK (MASK:1100,LONGWORD:2)"},
  {9,"WRITE MASK (MASK:0011,LONGWORD:3)"},
  {10,"WINDOW SPACE SETUP"},
  {11,"SLAVE RESPONSE"},
  {12,"29116 INSTRUCTION"},
  {32,"LESI REQUEST FLIP-FLOP NOT RESET"},
  {33,"LESI DATOB SIGNAL NOT RESET"},
  {34,"LESI NPR SIGNAL NOT RESET"},
  {35,"LESI INTR SIGNAL NOT RESET"},
  {36,"LESI DIR SIGNAL NOT RESET"},
  {37,"LESI SE SIGNAL NOT RESET"},
  {38,"LESI BLOCK MODE FLIP-FLOP NOT RESET"},
  {39,"LESI DATOB SIGNAL NOT SET"},
  {40,"LESI NPR SIGNAL NOT SET"},
  {41,"LESI INTR SIGNAL NOT SET"},
  {42,"LESI DIR SIGNAL NOT SET"},
  {43,"LESI SE SINAL NOT SET"},
  {44,"LESI REQUEST FLIP-FLOP NOT SET"},
  {45,"LESI BLK MOD FLIP/FLOP NOT SET"},
  {46,"LESI SE SIG NOT RESET"},
  {47,"LESI INTR SIG NOT RESET"},
  {48,"PATTERN 'AAAA' FAILED IN LAR"},
  {49,"PATTERN '5555' FAILED IN LAR"},
  {50,"HAR PATTERN FAILED OR BAD LESI PARITY"},
  {51,"HAR PATTERN FAILED OR BAD LESI PAR OR NO ATTN WITH LERR"},
  {52,"LESI STATUS REGISTER FAIL (ALL BITS SET)"},
  {53,"LESI STATUS REGISTER FAIL (ALL BITS RESET)"},
  {54,"NO RESET AFTER LESI WC OVF"},
  {55,"LESI RAM ADDRESS INTEGRITY"},
  {56,"NO SET AFTER LESI WC OVF"},
  {57,"READ VECTOR FROM LESI RAM FAILED"},
  {58,"LESI CD FAILED"},
  {59,"LESI RAM MSC PORT"},
  {60,"LESI RAM LESI PORT"} };
DD$FIELD_CODE  fc_669[] = { 
  {1,"REGDMP"} };
DD$FIELD_CODE  fc_670[] = { 
  {1,"BLA ERROR INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_671[] = { 
  {1,"INVALID MAP REGISTER"} };
DD$FIELD_CODE  fc_672[] = { 
  {1,"UNIBUS SSYN TIMEOUT"} };
DD$FIELD_CODE  fc_673[] = { 
  {1,"BI FAILURE"} };
DD$FIELD_CODE  fc_674[] = { 
  {1,"MAP REGISTER ERROR"} };
DD$FIELD_CODE  fc_675[] = { 
  {1,"ERROR"} };
DD$FIELD_CODE  fc_676[] = { 
  {1,"CABLE 0 GOOD TO BAD"},
  {2,"CABLE 1 GOOD TO BAD"},
  {3,"CABLE 0 BAD TO GOOD"},
  {4,"CABLE 1 BAD TO GOOD"},
  {5,"CABLES CROSSED TO UNCROSSED"},
  {6,"CABLES UNCROSSED TO CROSSED"},
  {7,"LOCAL PORT POWER UP"},
  {8,"INSUFFICIENT MEMORY FOR PORT INIT"},
  {9,"SCS_SYS_ID SET TO NON-ZERO"},
  {10,"CI ADPTR NOT AVAILABLE"},
  {11,"CAN'T FIND CI PORT UCODE"},
  {12,"BAD CPU UCODE REV"},
  {13,"UCODE VERIF ERROR"},
  {14,"FAILED START OF CI PORT UCOD"},
  {1,"FAILURE TO TALK DOWN"},
  {2,"ATTEMPT TO TALK UP"},
  {3,"REMOTE SYSTEM CONFLICTS W/ KNOWN SYSTEM"},
  {4,"RECEIVED CI PPD ERR DATAGRAM"},
  {1,"PATH CLOSING,ALL CABLES FAILED"},
  {2,"PATH CLOSING,BUFFER MEMORY SYS ERR"},
  {3,"PATH CLOSING,INVALID PACKET SIZE"},
  {4,"PATH CLOSING,UNRECOGNIZED PACKET"},
  {5,"PATH CLOSING,MESSAGE OUT OF SEQUENCE"},
  {6,"PATH CLOSING,RECEIVED MESSAGE ON CLOSED VCD"},
  {7,"PATH CLOSING,INVALID REMOTE PORT"},
  {1,"PATH CLOSING DUE TO SYSAP REQ"},
  {2,"PATH CLOSING DUE TO REMOTE SYSTEM REQUEST"},
  {3,"PATH CLOSING,CI PPD PROTOCOL ERROR"},
  {4,"PATH CLOSING,SCS FOUND BAD CONNID"},
  {5,"PATH CLOSING,SCS CONNECTION IN ILLEGAL STATE"},
  {6,"PATH CLOSING,SCS PROTOCOL TIMEOUT"},
  {7,"PATH CLOSING,BAD SCS CREDIT WITHDRAWAL"},
  {8,"PATH CLOSING,BAD SCS MESSAGE"},
  {9,"PATH CLOSINGG,UNEXPECTED BLOCK DATA TRANSFER"},
  {10,"PATH CLOSING,MESSAGE TRANSMITTED WITH NO SCS CREDITS"},
  {1,"PORT CRASHING,COMMAND QUEUE 0 INSERT FAILED"},
  {2,"PORT CRSAHING,COMMAND QUEUE 1 INSERT FAILED"},
  {3,"PORT CRASHING,COMMAND QUEUE 2 INSERT FAILED"},
  {4,"PORT CRASHING,COMMAND QUEUE 3 INSERT FAILED"},
  {5,"PORT CRASHING,DATAGRAM FREE QUEUE INSERT FAILED"},
  {6,"PORT CRASHING,MESSAGE FREE QUEUE INSERT FAILED"},
  {7,"PORT CRASHING,RESPONSE QUEUE REMOVE FAILED"},
  {8,"PORT CRASHING,DATAGRAM FREE QUEUE REMOVE FAILED"},
  {9,"PORT CRASHING,MESSAGE FREE QUEUE REMOVE FAILED"},
  {10,"PORT CRASHING,INVALID LOCAL BUFFER NAME"},
  {11,"PORT CRASHING,INVALID LOCAL BUFFER LENGTH"},
  {12,"PORT CRASHING,LOCAL BUFFER ACCESS VIOLATION"},
  {13,"PORT CRASHING,INVALID PACKET SIZE"},
  {14,"PORT CRASHING,INVALID DESTINATION PORT IN PACKET"},
  {15,"PORT CRASHING,UNKNOWN LOCAL PORT COMMAND"},
  {16,"PORT CRASHING,ABORTED PACKET"},
  {17,"PORT CRASHING,UNKNOWN STATUS IN PACKET"},
  {18,"PORT CRASHING,UNKNOWN OPCOD IN PACKET"},
  {19,"PORT CRASHING,INVALID OPCOD IN PACKET"},
  {20,"PORT CRASHING,CI ADAPTER UNAVAILABLE"},
  {21,"PORT CRASHING,MEMORY SYSTEM ERROR"},
  {22,"PORT CRASHING,BI MEMORY SYSTEM ERROR"},
  {23,"PORT CRASHING,DATA STRUCTURE ERROR"},
  {24,"PORT CRASHING,PARITY ERROR"},
  {25,"PORT CRASHG,BI PARITY ERR"},
  {26,"PORT CRASHING,PORT ERROR BITS SET"},
  {27,"PORT CRASHING,BI ERROR BITS SET"},
  {28,"PORT CRASHING,LOCAL PORT SANITY TIMER EXPIRED"},
  {29,"PORT CRASHING,MESSAGE FREE QUEUE EXHAUSTED"},
  {1,"PORT CRASHING,BAD PATH FOR SCS PACKET"},
  {2,"PORT CRASHING,LOCAL PORT LOST POWER"},
  {3,"PORT CRASHING,LOCAL PORT BROKEN"} };
DD$FIELD_CODE  fc_677[] = { 
  {1,"POWER FAIL DISABLE"} };
DD$FIELD_CODE  fc_678[] = { 
  {1,"TRANSMIT DCLO"} };
DD$FIELD_CODE  fc_679[] = { 
  {1,"TRANSMIT ACLO"} };
DD$FIELD_CODE  fc_680[] = { 
  {1,"NO CIPA"} };
DD$FIELD_CODE  fc_681[] = { 
  {1,"CIPA TIMEOUT"} };
DD$FIELD_CODE  fc_682[] = { 
  {1,"DIAGNOSE"} };
DD$FIELD_CODE  fc_683[] = { 
  {1,"CIPA PARITY ERR"} };
DD$FIELD_CODE  fc_684[] = { 
  {1,"CRD"} };
DD$FIELD_CODE  fc_685[] = { 
  {1,"UNCORRECTABLE READ DATA ERROR"} };
DD$FIELD_CODE  fc_686[] = { 
  {1,"READ LOCK TIMEOUT"} };
DD$FIELD_CODE  fc_687[] = { 
  {1,"NXM"} };
DD$FIELD_CODE  fc_688[] = { 
  {1,"ADAPTER POWER UP"} };
DD$FIELD_CODE  fc_689[] = { 
  {1,"ADAPTER POWER DOWN"} };
DD$FIELD_CODE  fc_690[] = { 
  {1,"MAINTENANCE INIT"} };
DD$FIELD_CODE  fc_691[] = { 
  {1,"MAINTENANCE TIMER DISABLED"} };
DD$FIELD_CODE  fc_692[] = { 
  {1,"MAINTENANCE INTERRUPT ENAABLE"} };
DD$FIELD_CODE  fc_693[] = { 
  {1,"MAINTENANCE INTERRUPT"} };
DD$FIELD_CODE  fc_694[] = { 
  {1,"WRONG PARITY"} };
DD$FIELD_CODE  fc_695[] = { 
  {1,"PROGRAM START ADDRESS"} };
DD$FIELD_CODE  fc_696[] = { 
  {1,"PORT UNINITIALIZED"} };
DD$FIELD_CODE  fc_697[] = { 
  {1,"TRANSMIT BUFFER PE DETECTED FROM IPB BOARD"} };
DD$FIELD_CODE  fc_698[] = { 
  {1,"OUTPUT PE"} };
DD$FIELD_CODE  fc_699[] = { 
  {1,"CIPA BUS PE"} };
DD$FIELD_CODE  fc_700[] = { 
  {1,"TRANSMIT BUFFER PE DETECTED FROM ILI BOARD"} };
DD$FIELD_CODE  fc_701[] = { 
  {1,"RECEIVE BUFFER PE"} };
DD$FIELD_CODE  fc_702[] = { 
  {1,"LOCAL STORE PE"} };
DD$FIELD_CODE  fc_703[] = { 
  {1,"CONTROL STORE PE"} };
DD$FIELD_CODE  fc_704[] = { 
  {1,"PARITY ERROR"} };
DD$FIELD_CODE  fc_705[] = { 
  {1,"RESPONSE QUEUE AVAILABLE"} };
DD$FIELD_CODE  fc_706[] = { 
  {1,"MESSAGE FREE QUEUE EMPTY"} };
DD$FIELD_CODE  fc_707[] = { 
  {1,"PORT DISABLE COMPLETE"} };
DD$FIELD_CODE  fc_708[] = { 
  {1,"PORT INIT COMPLETE"} };
DD$FIELD_CODE  fc_709[] = { 
  {1,"DATA STRUCTURE ERROR"} };
DD$FIELD_CODE  fc_710[] = { 
  {1,"MEM SYSTEM ERROR"} };
DD$FIELD_CODE  fc_711[] = { 
  {1,"SANITY TIMER EXPIRED"} };
DD$FIELD_CODE  fc_712[] = { 
  {1,"MAINTENANCE ERROR"} };
DD$FIELD_CODE  fc_713[] = { 
  {0,"16"},
  {1,"224"} };
DD$FIELD_CODE  fc_714[] = { 
  {0,"DUAL ROUND ROBIN"},
  {1,"FIXED-HI PRIORITY"},
  {2,"FIXED-LO PRIORITY"},
  {3,"DISABLE"} };
DD$FIELD_CODE  fc_715[] = { 
  {1,"SOFT ERROR INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_716[] = { 
  {1,"HARD ERROR INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_717[] = { 
  {1,"UNLOCK WRITE PENDING"} };
DD$FIELD_CODE  fc_718[] = { 
  {1,"NODE RESET"} };
DD$FIELD_CODE  fc_719[] = { 
  {1,"SELF TEST PASSED"} };
DD$FIELD_CODE  fc_720[] = { 
  {1,"SELF TEST CONTINUING"} };
DD$FIELD_CODE  fc_721[] = { 
  {1,"INIT IN PROGRESS"} };
DD$FIELD_CODE  fc_722[] = { 
  {1,"SOFT ERROR SUMMARY"} };
DD$FIELD_CODE  fc_723[] = { 
  {1,"HARD ERROR SUMMARY"} };
DD$FIELD_CODE  fc_724[] = { 
  {1,"NBIA PE"} };
DD$FIELD_CODE  fc_725[] = { 
  {1,"ENABLE LOOPBACK"} };
DD$FIELD_CODE  fc_726[] = { 
  {1,"FORCE NBIA PE"} };
DD$FIELD_CODE  fc_727[] = { 
  {1,"FORCE DMA BUSY"} };
DD$FIELD_CODE  fc_728[] = { 
  {1,"FLIP BITS 29 AND 22"} };
DD$FIELD_CODE  fc_729[] = { 
  {1,"NBI INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_730[] = { 
  {3,"READ DATA TIMEOUT"},
  {4,"NO RESPONSE TIMEOUT"},
  {5,"BUS ACCESS TIMEOUT"},
  {6,"INTERLOCK BUSY TIMEOUT"},
  {7,"BUSY TIMEOUT"} };
DD$FIELD_CODE  fc_731[] = { 
  {1,"DATA TRANSMIT DURING FAULT"} };
DD$FIELD_CODE  fc_732[] = { 
  {1,"WRITE DATA SEQUENCE FAULT"} };
DD$FIELD_CODE  fc_733[] = { 
  {1,"READ DATA SEQUENCE FAULT"} };
DD$FIELD_CODE  fc_734[] = { 
  {1,"CONTROL PARITY FAULT"} };
DD$FIELD_CODE  fc_735[] = { 
  {1,"DATA PARITY FAULT"} };
DD$FIELD_CODE  fc_736[] = { 
  {1,"NBIA PE"} };
DD$FIELD_CODE  fc_737[] = { 
  {1,"ENABLE LOOPBACK"} };
DD$FIELD_CODE  fc_738[] = { 
  {1,"FORCE NBIA PE"} };
DD$FIELD_CODE  fc_739[] = { 
  {1,"FORCE DMA BUSY"} };
DD$FIELD_CODE  fc_740[] = { 
  {1,"FLIP BITS 29 AND 22"} };
DD$FIELD_CODE  fc_741[] = { 
  {1,"NBI INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_742[] = { 
  {3,"READ DATA TIMEOUT"},
  {4,"NO RESPONSE TIMEOUT"},
  {5,"BUS ACCESS TIMEOUT"},
  {6,"INTERLOCK BUSY TIMEOUT"},
  {7,"BUSY TIMEOUT"} };
DD$FIELD_CODE  fc_743[] = { 
  {1,"DATA TRANSMIT DURING FAULT"} };
DD$FIELD_CODE  fc_744[] = { 
  {1,"WRITE DATA SEQUENCE FAULT"} };
DD$FIELD_CODE  fc_745[] = { 
  {1,"READ DATA SEQUENCE FAULT"} };
DD$FIELD_CODE  fc_746[] = { 
  {1,"CONTROL PARITY FAULT"} };
DD$FIELD_CODE  fc_747[] = { 
  {1,"DATA PARITY FAULT"} };
DD$FIELD_CODE  fc_748[] = { 
  {1,"BI0 PRESENT"} };
DD$FIELD_CODE  fc_749[] = { 
  {1,"BI 1 PRESENT"} };
DD$FIELD_CODE  fc_750[] = { 
  {1,"NBIB BI1 PE"} };
DD$FIELD_CODE  fc_751[] = { 
  {1,"NBIB BI0 PE"} };
DD$FIELD_CODE  fc_752[] = { 
  {1,"NBIA FUNCTION PE"} };
DD$FIELD_CODE  fc_753[] = { 
  {1,"BI0 POWER UP"} };
DD$FIELD_CODE  fc_754[] = { 
  {1,"BI1 POWER UP"} };
DD$FIELD_CODE  fc_755[] = { 
  {1,"NBIA WRAPAROUND"} };
DD$FIELD_CODE  fc_756[] = { 
  {1,"FORCE NBIB PE ON NBIB MODULE"} };
DD$FIELD_CODE  fc_757[] = { 
  {1,"NULL BUS PE"} };
DD$FIELD_CODE  fc_758[] = { 
  {1,"CORRECTED READ DATA"} };
DD$FIELD_CODE  fc_759[] = { 
  {1,"ID PARITY ERROR"} };
DD$FIELD_CODE  fc_760[] = { 
  {1,"USER PARITY ENABLE"} };
DD$FIELD_CODE  fc_761[] = { 
  {1,"ILLEGAL CONFIRMATION ERROR"} };
DD$FIELD_CODE  fc_762[] = { 
  {1,"NON-EXISTANT ADDRESS"} };
DD$FIELD_CODE  fc_763[] = { 
  {1,"BUS TIMEOUT"} };
DD$FIELD_CODE  fc_764[] = { 
  {1,"STALL TIMEOUT"} };
DD$FIELD_CODE  fc_765[] = { 
  {1,"RETRY TIMEOUT"} };
DD$FIELD_CODE  fc_766[] = { 
  {1,"READ DATA SUBSTITUTE"} };
DD$FIELD_CODE  fc_767[] = { 
  {1,"SLAVE PARITY ERR"} };
DD$FIELD_CODE  fc_768[] = { 
  {1,"COMMANDD PARITY ERROR"} };
DD$FIELD_CODE  fc_769[] = { 
  {1,"IDENT VECTOR ERR"} };
DD$FIELD_CODE  fc_770[] = { 
  {1,"TRANSMITTER DURING FAULT"} };
DD$FIELD_CODE  fc_771[] = { 
  {1,"INTERLOCK SEQUENCE ERROR"} };
DD$FIELD_CODE  fc_772[] = { 
  {1,"MASTER PARITY ERR"} };
DD$FIELD_CODE  fc_773[] = { 
  {1,"CONTROL TRANSMIT ERROR"} };
DD$FIELD_CODE  fc_774[] = { 
  {1,"MASTER TRANSMIT CHECK ERROR"} };
DD$FIELD_CODE  fc_775[] = { 
  {1,"NO ACK TO MULTI RESPONSE COMMAND"} };
DD$FIELD_CODE  fc_776[] = { 
  {1,"ILLEGAL SYSTEM VIRTUAL ADDRESS FORMAT"},
  {2,"NON-EXISTENT SYSTEM VIRTUAL ADDRESS"},
  {3,"INVALID SYSTEM PTE"},
  {4,"INVALID BUFFER PTE"},
  {5,"NON-EXISTENT SYSTEM, GLOBAL VIRTUAL ADDRESS"},
  {6,"NON-EXISTENT BUFFER, GLOBAL VIRTUAL ADDRESS"},
  {7,"INVALID SYSTEM GLOBAL PTE"},
  {8,"INVALID BUFFER GLOBAL PTE"},
  {9,"INVLD SYSTEM GLOBAL PTE MAPPING"},
  {10,"INVLD BUFFER GLOBAL PTE MAPPING"},
  {11,"QUEUE INTERLOCK RETRY FAILED"},
  {12,"ILLEGAL QUEUE OFFSET ALIGNMENT"},
  {13,"ILLEGAL PQB FORMAT"},
  {14,"REGISTER PROTOCOL VIOLATION"} };
DD$FIELD_CODE  fc_777[] = { 
  {1,"ERROR SUMMARY"} };
DD$FIELD_CODE  fc_778[] = { 
  {1,"RESPONSE QUEUED"} };
DD$FIELD_CODE  fc_779[] = { 
  {1,"BI TRANSACTION ERROR"},
  {2,"ADAPTER EXCEPTION"},
  {3,"NON-FATAL BI ERROR"},
  {4,"FATAL BI ERROR"},
  {5,"DATA STRUCTURE ERROR"},
  {6,"PORT LOGICAL ERROR"},
  {7,"ADAPTER HARD ERROR"} };
DD$FIELD_CODE  fc_780[] = { 
  {1,"UNDEFINED"},
  {2,"INITIALIZED"},
  {4,"ENABLED"},
  {6,"STOPPED"},
  {7,"MAINTENANCE"} };
DD$FIELD_CODE  fc_781[] = { 
  {1,"FREE QUEUE EXHAUSTED"} };
DD$FIELD_CODE  fc_782[] = { 
  {1,"ERROR LOST"} };
DD$FIELD_CODE  fc_783[] = { 
  {1,"EXTENDED SELF TEST PASSED"} };
DD$FIELD_CODE  fc_784[] = { 
  {1,"ADAPTER CAN COMMUNICATE"} };
DD$FIELD_CODE  fc_785[] = { 
  {1,"SELF TEST DONE"} };
DD$FIELD_CODE  fc_786[] = { 
  {1,"RESPONSE TO PORT STATUS QUERY"} };
DD$FIELD_CODE  fc_787[] = { 
  {0,"REGISTER WRITTEN BY HOST"},
  {1,"REGISTER WRITTEN BY ADAPTER"} };
DD$FIELD_CODE  fc_788[] = { 
  {1,"ADAPTER HARDWARE FAILURE"},
  {2,"ADAPTER BUGCHECK"},
  {3,"FAILED MINIMUM SELF TEST"},
  {4,"FAILED BI SPECIFIED SELF TEST"},
  {5,"FAILED EXTENDED SELF TEST"},
  {6,"FAILD UCODE LOADING"},
  {7,"UCODE NOT LOADED"},
  {8,"INVALID PQB CONTENTS"},
  {9,"QUEUE INTERLOCK RETRY"},
  {10,"ILELGAL QUEUE OFFSET"},
  {11,"ILLEGAL/RESERVED PORT INSTRUCTION"},
  {12,"ILLEGAL PORT INSTRUCTION FOR CURRENT PORT STATE"},
  {13,"ADAPTER TIMEOUT WHILE CLEARING PS OWNER BIT"},
  {14,"ADAPTER TIMEOUT,HOST INACTIVE"},
  {15,"FREE QUEUE EXHAUSTD"},
  {16,"FAILED TO ENTER MAINT STATE"},
  {17,"FAILED TO ENTER INITIALIZED STATE"},
  {18,"FAILED TO ENTER ENABLED STATE"},
  {19,"BUFFER KEY MISMATCH"},
  {20,"SIZE OF DATA TRANSFER LARGER THAN BUFFER SIZE"},
  {21,"BUFFER ACCESS CHECK ERROR"},
  {22,"BAD PTE"},
  {23,"BUFFER INDEX OUT OF RANGE"} };
DD$FIELD_CODE  fc_789[] = { 
  {1,"READ MASKED"},
  {2,"WRITE MASKED"},
  {4,"INTERLOCK READ MASK"},
  {7,"INTERLOCK WRITE MASK"},
  {8,"EXTENDED READ MASK"},
  {11,"EXTENDED WRITE MASK"} };
DD$FIELD_CODE  fc_790[] = { 
  {1,"NO CIPA"} };
DD$FIELD_CODE  fc_791[] = { 
  {1,"CIPA DCLO"} };
DD$FIELD_CODE  fc_792[] = { 
  {1,"DIAGNOSTIC ENABLED"} };
DD$FIELD_CODE  fc_793[] = { 
  {1,"POWER UP"} };
DD$FIELD_CODE  fc_794[] = { 
  {1,"POWER DOWN"} };
DD$FIELD_CODE  fc_795[] = { 
  {1,"BI BUSY ERROR"} };
DD$FIELD_CODE  fc_796[] = { 
  {1,"BI PARITY ERROR"} };
DD$FIELD_CODE  fc_797[] = { 
  {1,"BIC ADAPTER PARITY ERROR"} };
DD$FIELD_CODE  fc_798[] = { 
  {1,"CIPA BUS PARITY ERROR"} };
DD$FIELD_CODE  fc_799[] = { 
  {1,"START"} };
DD$FIELD_CODE  fc_800[] = { 
  {1,"MAINTENANCE TIMER DISABLE"} };
DD$FIELD_CODE  fc_801[] = { 
  {1,"WRONG PARITY"} };
DD$FIELD_CODE  fc_802[] = { 
  {1,"MAINTENANCE INTERRUPT ENABLE"} };
DD$FIELD_CODE  fc_803[] = { 
  {1,"HALT SEQUENCER"} };
DD$FIELD_CODE  fc_804[] = { 
  {1,"BI BUS TIMEOUT"} };
DD$FIELD_CODE  fc_805[] = { 
  {1,"II PARITY ERROR"} };
DD$FIELD_CODE  fc_806[] = { 
  {1,"MAP BCI/BI ERROR"} };
DD$FIELD_CODE  fc_807[] = { 
  {1,"CILP PARITY ERROR"} };
DD$FIELD_CODE  fc_808[] = { 
  {1,"TRANSMIT BUFFER PARITY ERROR"} };
DD$FIELD_CODE  fc_809[] = { 
  {1,"INTERNAL BUS PARITY ERROR"} };
DD$FIELD_CODE  fc_810[] = { 
  {1,"CONTROL STORE PARITY ERROR"} };
DD$FIELD_CODE  fc_811[] = { 
  {1,"BCI PARITY ERROR"} };
DD$FIELD_CODE  fc_812[] = { 
  {1,"RESPONSE QUEUE AVAILABLE"} };
DD$FIELD_CODE  fc_813[] = { 
  {1,"MESSAGE FREE QUEUE EMPTY"} };
DD$FIELD_CODE  fc_814[] = { 
  {1,"PORT DISABLE COMPLETE"} };
DD$FIELD_CODE  fc_815[] = { 
  {1,"PORT INIT COMPLETE"} };
DD$FIELD_CODE  fc_816[] = { 
  {1,"DATA STRUCTURE ERROR"} };
DD$FIELD_CODE  fc_817[] = { 
  {1,"MEMORY SYSTEM ERROR"} };
DD$FIELD_CODE  fc_818[] = { 
  {1,"SANITY TIMER EXPIRED"} };
DD$FIELD_CODE  fc_819[] = { 
  {1,"MISCELLANEOUS ERROR"} };
DD$FIELD_CODE  fc_820[] = { 
  {1,"MAINTENANCE ERROR"} };
DD$FIELD_CODE  fc_821[] = { 
  {1,"MAINTENANCE INTERRUPT"} };
DD$FIELD_CODE  fc_822[] = { 
  {1,"PORT UNINITIALIZED"} };
DD$FIELD_CODE  fc_823[] = { 
  {1,"NO RESPONSE ERROR"} };
DD$FIELD_CODE  fc_824[] = { 
  {0,"ALTER DELTA TIME ON SLOT 7"},
  {1,"ALTER DELTA TIME ON SLOT 16"},
  {2,"ALTER DELTA TIME ON SLOT 32"},
  {3,"ALTER DELTA TIME ON SLOT 64"} };
DD$FIELD_CODE  fc_825[] = { 
  {1,"EXTEND HEADER"} };
DD$FIELD_CODE  fc_826[] = { 
  {1,"DISABLE ARBITRATION"} };
DD$FIELD_CODE  fc_827[] = { 
  {0,"16"},
  {1,"32"},
  {2,"64"},
  {3,"128"},
  {8,"224"} };
DD$FIELD_CODE  fc_828[] = { 
  {1,"PORT INIT"},
  {2,"ENABLE QUEUE PROCESSING"},
  {3,"READ PIV"},
  {4,"PORT SHUTDOWN"},
  {5,"ENTER MAINT STATE"},
  {6,"COMMAND QUEUE NON-EMPTY"},
  {7,"FREE QUEUE NON-EMPTY"},
  {8,"PORT STATUS QUERY"},
  {9,"ENABLE TRANSACTION ERROR REPORTING"},
  {10,"DISABLE TRANSACTION ERROR REPORTING"},
  {11,"RESTART"} };
DD$FIELD_CODE  fc_829[] = { 
  {0,"HOST"},
  {1,"ADAPTER"} };
DD$FIELD_CODE  fc_830[] = { 
  {1,"PARITY ENABLE"} };
DD$FIELD_CODE  fc_831[] = { 
  {1,"WRITE WRONG PARITY"} };
DD$FIELD_CODE  fc_832[] = { 
  {1,"MEMORY SYSTEM LOST ERROR BIT"} };
DD$FIELD_CODE  fc_833[] = { 
  {1,"DMA Q22 BUS ADDRESS SPACE PARITY ERROR"} };
DD$FIELD_CODE  fc_834[] = { 
  {1,"CPU Q22 BUS ADDRESS SPACE PARITY ERROR"} };
DD$FIELD_CODE  fc_835[] = { 
  {1,"CPU LOCAL ADDRESS SPACE PARITY ERROR"} };
DD$FIELD_CODE  fc_836[] = { 
  {1,"CPU NON-EXISTANT MEMORY ERROR"} };
DD$FIELD_CODE  fc_837[] = { 
  {0,"Q22-BUS MEM / DEV"},
  {1,"KA630-A ON-BOARD MEM"},
  {2,"MEM EXP MOD 1"},
  {3,"MEM EXP MOD 2"} };
DD$FIELD_CODE  fc_838[] = { 
  {1,"SYSTEM RESET"},
  {0,"RESET BY NODE RESET"} };
DD$FIELD_CODE  fc_839[] = { 
  {1,"PARITY ENABLED"} };
DD$FIELD_CODE  fc_840[] = { 
  {0,"TOY CHECKSUM BAD"} };
DD$FIELD_CODE  fc_841[] = { 
  {1,"MEMORY LOOP"} };
DD$FIELD_CODE  fc_842[] = { 
  {1,"MEMORY LOST"} };
DD$FIELD_CODE  fc_843[] = { 
  {1,"PARITY ENABLE"} };
DD$FIELD_CODE  fc_844[] = { 
  {1,"WRITE WRONG PARITY"} };
DD$FIELD_CODE  fc_845[] = { 
  {1,"PARITY ERROR"} };
DD$FIELD_CODE  fc_846[] = { 
  {1,"MEMORY CODE 0"} };
DD$FIELD_CODE  fc_847[] = { 
  {-32767,"PACKET READ ERROR"},
  {-32766,"PACKET WRITE ERROR"},
  {-32765,"ROM/RAM PARITY ERROR"},
  {-32764,"RAM PARITY ERROR"},
  {-32763,"ROM PARITY ERROR"},
  {-32762,"RING READ ERROR"},
  {-32761,"RING WRITE ERROR"},
  {-32760,"INTERRUPT MASTER ERROR"},
  {-32759,"HOST ACCESS TIMEOUT ERROR"},
  {-32758,"HOST EXCEED COMMAND ERROR"},
  {-32757,"BUS MASTER FAIL"},
  {-32756,"DM XFC FATAL ERROR"},
  {-32755,"CONTROLLERR HARDWARE ERROR"},
  {-32754,"INVALID V/C ID"},
  {-32753,"INTERRUPT WRITE ERROR"},
  {-32749,"TOO MANY SUBUNITS"},
  {-32746,"MAP REGISTER READ ERROR"},
  {-30720,"FATAL SEQUENCE ERROR"},
  {-30688,"D PROC ALU ERR"},
  {-30687,"D PROC CTRL ROM PE"},
  {-30142,"D PROC NO BOARD 2/RAM PE"},
  {-30139,"D PROC RAM BUFFER ERROR"},
  {-30102,"D PROC SDI ERROR"},
  {-30101,"D PROC WRITE MODE WRAP SERDES"},
  {-30100,"D PROC READ MODE SERDES/RSGEN&ECC ERR"},
  {-29664,"U PROC ALU ERR"},
  {-29663,"U PROC CTRL REG ERR"},
  {-29662,"U PROC DFAIL/CROM PE"},
  {-29657,"U PROC CONSTANT PROM ERR DURING SDI TEST"},
  {-29651,"UNEXPECTED TRAP"},
  {-29639,"U PROC CONSTANT PROM ERR"},
  {-29638,"U PROC CROM PE"},
  {-29568,"STEP 1 DATA ERROR (MSB NOT SET)"},
  {-29117,"U PROC RAM PE"},
  {-29113,"U PROC RAM BUFFER ERROR"},
  {-29107,"BOARD 2 TEST CNT WRONG"},
  {-27456,"STEP 2 ERROR"},
  {-23422,"NPR ERROR"},
  {-23360,"STEP 3 ERROR"},
  {-15168,"STEP 4 ERROR"} };
DD$FIELD_CODE  fc_848[] = { 
  {-32668,"BI MASTER ERROR"},
  {-32667,"KDB BIIC DETECTED ERROR"},
  {-32666,"KDB DETECTED BI STOP COMMAND"},
  {-30720,"FATAL SEQUENCE ERROR"},
  {-30688,"D PROC ALU TEST ERROR"},
  {-30687,"D PROC ROM PE"},
  {-30142,"D PROC-NO BOARD 2/RAM PE"},
  {-30139,"D PROC RAM BUFFER ERROR"},
  {-30102,"D PROC SDI ERROR"},
  {-30101,"D PROC WRITE MODE WRAP SERDES"},
  {-30100,"D PROC READ MODE SERDES/RSGEN&ECC ERR"},
  {-29664,"U PROC ALU ERROR"},
  {-29663,"U PROC CONTROL REGISTER ERROR"},
  {-29662,"U PROC PE SET ERRONEOUSLY"},
  {-29651,"UNEXPECTED TRAP"},
  {-29639,"U PROC LOG/ANTILOG RAM CHECKSUM ERROR"},
  {-29638,"U PROC ROM PE"},
  {-29568,"STEP 1 ERROR"},
  {-29472,"BI 10 SECOND COMMAND TIMEOUT/FAILED BIIC SELF TEST"},
  {-29471,"BCAI BUFFER ERROR (LOOPBACK MODE)"},
  {-29470,"BI PARITY TEST ERROR (LOOPBACK MODE)"},
  {-29469,"BIIC BUFFER ERROR (LOOPBACK MODE)"},
  {-29468,"BIIC BUFFER ERROR (NORMAL MODE)"},
  {-29467,"POLL TEST ERROR (NORMAL MODE)"},
  {-29441,"BI MASTER ERROR"},
  {-29117,"U PROC RAM PE"},
  {-29113,"U PROC RAM BUFFER ERROR"},
  {-23392,"DMA TEST ERROR"} };
DD$FIELD_CODE  fc_849[] = { 
  {-32767,"PACKET READ ERROR"},
  {-32766,"PACKET WRITE ERROR"},
  {-32762,"QUEUE READ ERROR"},
  {-32761,"QUEUE WRITE ERROR"},
  {-32759,"HOST ACCESS TIMEOUT"},
  {-32754,"INVALID CONNECT ID"},
  {-32748,"PROTOCOL INCOMPLETE ERROR"},
  {-32747,"PURGE/POLL FAILURE"},
  {-32746,"MAP READ ERROR"},
  {-32317,"BEGINNING POWER UP SELF TEST FAILED"},
  {-32316,"ROM CHECKSUM TEST FAILED"},
  {-32315,"QBUS DMA POINTER (LOW)-READ ONLY FAILED"},
  {-32314,"QBUS DMA POINTER (HI)-READ ONLY FAILED"},
  {-32313,"QBUS INTERRUPT VCTR TEST-READ ONLY FAILED"},
  {-32312,"QBUS DMA CSR-READ ONLY TEST FAILED"},
  {-32311,"QBUS DMA WORD COUNT-READ ONLY TST FAILED"},
  {-32310,"DATA RCOVERY CSR-READ ONLY TEST FAIL"},
  {-32309,"MEMORY DMA PTR-READ ONLY TEST FAILED"},
  {-32308,"RAM TEST FAILED"},
  {-32307,"CHECK FOR CLIP TEST FAILED"},
  {-32306,"BEGIN PORT INIT SELFTEST FAILED"},
  {-32305,"QBUS DMA CSR-READ/WRITE TEST FAILED"},
  {-32304,"MEMORY DMA POINTER-READ/WRITE TEST FAILED"},
  {-32303,"DATA RECVERY CSR-READ/WRITE TEST FAILED"},
  {-32302,"QBUS DMA WORD COUNT-READ/WRITE TEST FAIL"},
  {-32301,"SMC9224 REG FILE POINTER-READ/WRITE FAIL"},
  {-32300,"SMC9224 REG FILE VAL-READ/WRITE FAILED"},
  {-32299,"SMC9224 INTERRUPT"},
  {-32298,"RAM TEST (NOT T-11 ONLY) FAILED"},
  {-32293,"ILLEGAL INTERUPT (T-11 FAILED)"},
  {-32292,"DUP SEND DATA FAILURE"},
  {-32291,"DUP RECV DATA FAILURE"} };
DD$FIELD_CODE  fc_850[] = { 
  {0,"MEMORY CONTROLLER BUGCHECK"},
  {1,"UNRECOVERABLE READ ERROR"},
  {2,"NON-EXISTENT MEMORY"},
  {3,"UNALIGNED REFERENCE TO I/O SPACE"},
  {4,"PAGE TABLE READ ERROR"},
  {5,"PAGE TABLE WRITE ERROR"},
  {6,"CONTROL STORE PE"},
  {7,"MICROMACHINE BUGCHK"},
  {8,"Q22 BUS VECTOR READ ERROR"},
  {9,"ERROR WRITING PARAMETER ON STACK"} };
DD$FIELD_CODE  fc_851[] = { 
  {1,"IMPOSSIBLE UCODE STATE (FSD)"},
  {2,"IMPOSSIBLE UCODE STATE (SSD)"},
  {3,"UNDEFINED FPU ERROR CODE 0"},
  {4,"UNDEFINED FPU ERROR CODE 7"},
  {5,"MEMORY MANAGEMENT ERROR STATUS (TB MISS)"},
  {6,"MEMORY MANAGEMENT ERROR STATUS (M = 0)"},
  {7,"PROCESS PTE ADDRESS IN P0 SPACE"},
  {8,"PROCESS PTE ADDRESS IN P1 SPACE"},
  {9,"UNDEFINED INTERRUPT ID CODE"},
  {128,"READ BUS ERROR, VAP IS VIRTUAL ADDRESS"},
  {129,"READ BUS ERROR, VAP IS PHYSICAL ADDRESS"},
  {130,"WRITE BUS ERROR, VAP IS VIRTUAL ADDRESS"},
  {131,"WRITE BUS ERROR, VAP IS PHYSICAL ADDRESS"} };
DD$FIELD_CODE  fc_852[] = { 
  {0,"UCODE ERRORS"},
  {1,"TRANS BUFFER PE"},
  {2,"UNDEFINED MCK CODE"},
  {3,"BAD MEMORY CSR FORMAT"},
  {4,"ILLEGAL INTERRUPT"},
  {5,"FPA PARITY ERROR"},
  {6,"SYSTEM PTE READ ERROR"},
  {7,"RDS"},
  {8,"NXM"},
  {9,"UNALIGNED REFERENCE TO I/O SPACE"},
  {10,"UNRECOGNIZED I/O SPACE ADDRESS"},
  {11,"UNALIGNED REFERENCE TO UNIBUS SPACE"} };
DD$FIELD_CODE  fc_853[] = { 
  {1,"PORT B RTDS ENABLED"} };
DD$FIELD_CODE  fc_854[] = { 
  {1,"PORT A RTDS ENABLED"} };
DD$FIELD_CODE  fc_855[] = { 
  {1,"PORT B ENABLED"} };
DD$FIELD_CODE  fc_856[] = { 
  {1,"PORT A ENABLED"} };
DD$FIELD_CODE  fc_857[] = { 
  {1,"AVAILABLE ASSERTED"} };
DD$FIELD_CODE  fc_858[] = { 
  {1,"ATTENTION ASSERTED"} };
DD$FIELD_CODE  fc_859[] = { 
  {1,"READ/WRITE READY ASSERTED"} };
DD$FIELD_CODE  fc_860[] = { 
  {1,"RECEIVER READY ASSERTED"} };
DD$FIELD_CODE  fc_861[] = { 
  {1,"DRIVE AVAILABLE"} };
DD$FIELD_CODE  fc_862[] = { 
  {1,"DRIVE ON-LINE"} };
DD$FIELD_CODE  fc_863[] = { 
  {1,"RTDS ATTENTION"} };
DD$FIELD_CODE  fc_864[] = { 
  {1,"INTERNAL READ/WRITE READY"} };
DD$FIELD_CODE  fc_865[] = { 
  {1,"SOFT ERROR"} };
DD$FIELD_CODE  fc_866[] = { 
  {1,"HARD ERROR"} };
DD$FIELD_CODE  fc_867[] = { 
  {-32767,"ENVELOP/PACKET READ ERROR"},
  {-32766,"ENVELOP/PACKET WRITE ERROR"},
  {-32765,"ROM/RAM PE"},
  {-32764,"RAM PE"},
  {-32763,"ROM PE"},
  {-32762,"QUEUE READ ERROR"},
  {-32761,"QUEUE WRITE ERROR"},
  {-32760,"INTERRUPT MASTER ERROR"},
  {-32759,"HOST ACCESS TIMEOUT"},
  {-32758,"CREDIT LIMIT EXCEEDED"},
  {-32757,"BUS MASTER ERROR"},
  {-32756,"DIAG CONTROLLER FATAL ERROR"},
  {-32755,"INSTRUCTION LOOP TIMEOUT"},
  {-32754,"INVALID CONNECTION ID"},
  {-32753,"INTERRUPT WRITE ERROR"},
  {-32752,"MAINTENANCE READ/WRITE INVALID REGION ID"},
  {-32751,"MAINTENANCE WRITE LOAD TO NON-LOADABLE CONTROLLER"},
  {-32750,"RAM ERROR (NON-PARITY)"},
  {-32749,"INIT SEQUENCE ERROR"},
  {-32748,"HI LEVEL PROTOCOL INCOMPATIBILITY ERROR"},
  {-32747,"PURGE/POLL HARDWARE FAILURE"},
  {-32746,"MAP REGISTER READ ERROR"},
  {-32745,"DATA TRANSFER MAPPING WITH NO OPTION PRESENT"},
  {-32168,"DIVIDE INTERRUPT ERROR"},
  {-32167,"SINGLE INTERRUPT ERROR"},
  {-32166,"NON-MASKABLE INTERRUPT ERROR"},
  {-32165,"BREAKPOINT INTERRUPT ERROR"},
  {-32164,"INTO DETECTED INTERRUPT ERROR"},
  {-32163,"ARRAY BOUND INTERRUPT ERROR"},
  {-32162,"UNUSED OPCOD INTERRUPT ERROR"},
  {-32161,"ESC OPCOD INTERRUPT ERROR"},
  {-32159,"RESERVED INTERRUPT ERROR"},
  {-32156,"INTO INTERRUPT ERROR"},
  {-32143,"ROM CHECKSUM ERROR"},
  {-32142,"MPU ERROR"},
  {-32141,"RAM ERROR (ODD BYTE)"},
  {-32140,"RAM ERROR (EVN BYTE)"},
  {-32139,"MPU TIMER ERR"},
  {-32138,"MISC REGISTER WRAP ERR"},
  {-32137,"GAP DETECTED CIRCUIT ERR"},
  {-32136,"USART WRAP MODE ERROR"},
  {-32135,"USART WRP MODE ERROR (GD CRC)"},
  {-32134,"USART WRP MODE ERROR (BAD CRC)"},
  {-32133,"DRIVE CABLE ERROR"},
  {-32132,"FPLS BUFFER ERROR - NIBBLE 1"},
  {-32131,"FPLS BUFFER ERROR - NIBBLE 2"},
  {-32130,"FPLS BUFFER ERROR - NIBBLE 3"},
  {-32129,"FPLS BUFFER ERROR - NIBBLE 4"},
  {-32128,"WORD COUNT ERR"},
  {-32127,"FPLS TEST ERR"},
  {-32117,"RESERVED - ECC"},
  {-32116,"WRITE SEQUENCE FAULT"},
  {-32115,"ECC LOGIC ERROR TYPE 1"},
  {-32114,"ECC LOGIC ERROR TYPE 2"},
  {-32113,"ECC DATA STRUCTURE CONSISTENCY FAILURE"} };
DD$FIELD_CODE  fc_868[] = { 
  {-32767,"ENVELOP/PACKET READ ERROR"},
  {-32766,"ENVELOP/PACKET WRITE ERROR"},
  {-32765,"ROM/RAM PE"},
  {-32764,"RAM PE"},
  {-32763,"ROM PE"},
  {-32762,"QUEUE READ ERROR"},
  {-32761,"QUEUE WRITE ERROR"},
  {-32760,"INTERRUPT MASTER ERROR"},
  {-32759,"HOST ACCESS TIMEOOUT"},
  {-32758,"CREDIT LIMIT EXCEEDED"},
  {-32757,"BUS MASTER ERROR"},
  {-32756,"DIAGNOSTIC CONTROLLER FATAL ERROR"},
  {-32755,"INSTRUCTION LOOP TIMEOUT"},
  {-32754,"INVALID CONNECTION ID"},
  {-32753,"INTERRUPT WRITE ERROR"},
  {-32752,"MAINTENANCE READ/WRITE INVALID REGION ID"},
  {-32751,"MAINTENANCE WRITE LOAD TO NON-LOADABLE CONTROLLERR"},
  {-32750,"RAM ERROR (NON-PARITY)"},
  {-32749,"INIT SEQUENCE ERROR"},
  {-32748,"HI LEVEL PROTOCOL INCOMPATIBILITY ERROR"},
  {-32747,"PURGE/POLL HARDWARE FAILURE"},
  {-32746,"MAP REGISTER READ ERROR"},
  {-32745,"DATA TRANSFER MAPPING WITH NO OPTION PRESENT"},
  {-32696,"READ/WRITE ERROR ON INTERRUPT"},
  {-32695,"INCONSSTNCY AT 'U.BFIL'"},
  {-32694,"INCONSISTENCY AT 'U.BMTY'"},
  {-32693,"INCONSISTENCY AT 'U.ALOC'"},
  {-32692,"INVALID SERVO ENTRY (PIP SET)"},
  {-32691,"INVALID AT SERVO ENTRY (ERR SET)"},
  {-32690,"INCONSISTENCY AT 'U.SEND'"},
  {-32689,"INCONSISTENCY AT 'U.RECV'"},
  {-32688,"INCONSISTENCY AT 'U.ATTN'"},
  {-32687,"INCONSISTENCY AT 'U.ONLN'"},
  {-32686,"ILLEGAL D REQUEST (U.QDRQ)"},
  {-32685,"FENCE POST ERROR AT 'PROTAB'"},
  {-32684,"BAD PACKET DEQUEUED AT 'U.DONE'"},
  {-32683,"'DM' PROGRAM ILLEGAL MEMORY STORE"},
  {-32682,"'DUP' D-Q FAILED (XFC 34/35)"},
  {-32681,"INCONSISTENCY AT 'U.HTST'"},
  {-32680,"INCONSISTENCY AT 'U.SEKO'"},
  {-32679,"INCONSSTNCY AT 'U.CKSV'"},
  {-32678,"'D.OPCD' FOUND ILLEGAL OPCOD"},
  {-32677,"'D.CSF' FOUND ILLEGAL OPCOD"},
  {-32676,"UNKNOWN BAD DRIVE STATUS, 'D.DSTS'"},
  {-32675,"ILLGL 'XFC' EXECUTED BY 'DM'"},
  {-32674,"'D' PICKED UP A ZERO 'SCB.DB'"},
  {-32673,"INCONSISTENCY AT 'D' IDLE LOOP"},
  {-32672,"'DM' WRD COUNT ERR"},
  {-32671,"UNKNOWN DISPLAY FAULT, 'D.DFLT'"},
  {-32670,"DRIVE NOT FAULTING, 'P.OFLN' STATE"},
  {-32669,"'U' POWER UP DIAGNOSTICS FAILED"},
  {-32668,"'D' POWER UP DIAGNOSTICS FAILED"},
  {-32667,"ADAPTER CARD FAILURE"},
  {-32666,"'EC.TMR' TIMED OUT"},
  {-32665,"'U.SEND/U.RECV' RING READ TIMEOUT"},
  {-32664,"'WAITRV' REASON AT 'D.RVCT'"},
  {-32663,"'D.ARCS,' CLOSEST UNDONE ZONE LOST"},
  {-32662,"'U.SEEK', SEEK TO ILLEGAL TRACK"},
  {-32661,"'U.HTST', INIT DIAGNOSTIC WRITE FAILED"},
  {-32660,"'U.HTST', INIT DIAGNOSTIC DMA FAILED"},
  {-32659,"'U.SYDR'-'SS.DER' 1, 'SS.SPN' 0"},
  {-32658,"MASTER DRIVE ACLO ASSERTED"} };

/*************** REGISTER_FIELD_STRUCT ***************/

DD$REGISTER_FIELD_DSD  rf_eventtype[] = { 
  {32,3,1,21,fc_1," "} };
DD$REGISTER_FIELD_DSD  rf_m86ehm[] = { 
  {4,3,1,4,fc_2,"ERROR CODE"},
  {1,3,1,1,fc_3," "},
  {1,3,1,1,fc_4," "},
  {1,3,1,1,fc_5," "},
  {1,3,1,1,fc_6," "},
  {8,1,3,0,0,"UTRAP VECTOR ADDR:"},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_7," "},
  {1,3,1,1,fc_8," "},
  {1,3,1,1,fc_9," "},
  {1,3,1,1,fc_10," "},
  {1,3,1,1,fc_11," "},
  {1,3,1,1,fc_12," "},
  {1,3,1,1,fc_13," "},
  {1,3,1,1,fc_14," "},
  {1,3,1,1,fc_15," "},
  {1,3,1,1,fc_16," "},
  {1,3,1,1,fc_17," "},
  {1,3,1,1,fc_18," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_19," "},
  {1,3,1,1,fc_20," "} };
DD$REGISTER_FIELD_DSD  rf_m86ebcs[] = { 
  {1,7,1,0,0," "},
  {1,3,1,1,fc_21," "},
  {1,3,1,1,fc_22," "},
  {1,3,1,1,fc_23," "},
  {1,3,1,1,fc_24," "},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_25," "},
  {1,3,1,1,fc_26," "},
  {1,3,1,1,fc_27," "},
  {1,3,1,1,fc_28," "},
  {1,3,1,1,fc_29," "},
  {1,3,1,1,fc_30," "},
  {1,3,1,1,fc_31," "},
  {1,3,1,1,fc_32," "},
  {4,7,1,0,0," "},
  {1,3,1,1,fc_33," "},
  {6,7,1,0,0," "},
  {1,3,1,1,fc_34," "},
  {1,3,1,1,fc_35," "},
  {1,3,1,1,fc_36," "},
  {1,3,1,1,fc_37," "},
  {1,3,1,1,fc_38," "} };
DD$REGISTER_FIELD_DSD  rf_m86edpsr[] = { 
  {1,3,1,1,fc_39," "},
  {1,3,1,1,fc_40," "},
  {1,3,1,1,fc_41," "},
  {1,3,1,1,fc_42," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_43," "},
  {1,3,1,1,fc_44," "},
  {1,3,1,1,fc_45," "},
  {1,3,1,1,fc_46," "},
  {2,7,1,0,0," "},
  {1,3,1,1,fc_47," "},
  {4,1,2,0,0,"VMQ ERROR BYTE:"},
  {8,7,1,0,0," "},
  {4,1,2,0,0,"AMUX ERROR BYTE:"},
  {4,1,2,0,0,"BMUX ERROR BYTE:"} };
DD$REGISTER_FIELD_DSD  rf_m86ibesr[] = { 
  {8,7,1,0,0," "},
  {2,3,1,4,fc_48,"OPBUS DATA SRC:"},
  {1,3,1,2,fc_49,"OPBUS SOURCE:"},
  {3,3,1,8,fc_50," "},
  {1,3,1,1,fc_51," "},
  {6,7,1,0,0," "},
  {1,3,1,1,fc_52," "},
  {1,3,1,1,fc_53," "},
  {1,3,1,1,fc_54," "},
  {1,3,1,1,fc_55," "},
  {1,3,1,1,fc_56," "},
  {1,3,1,1,fc_57," "},
  {1,3,1,1,fc_58," "},
  {1,3,1,2,fc_59,"IAMUX SOURCE:"},
  {2,1,2,0,0,"IBOX AMUX BYTE:"},
  {1,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m86ms1[] = { 
  {1,3,1,1,fc_60," "},
  {1,3,1,1,fc_61," "},
  {1,3,1,1,fc_62," "},
  {1,3,1,1,fc_63," "},
  {4,1,2,0,0,"BYTE IN ERROR:"},
  {1,3,1,1,fc_64," "},
  {1,3,1,1,fc_65," "},
  {1,3,1,1,fc_66," "},
  {1,3,1,1,fc_67," "},
  {1,3,1,1,fc_68," "},
  {1,3,1,1,fc_69," "},
  {1,3,1,1,fc_70," "},
  {1,3,1,1,fc_71," "},
  {2,1,2,0,0,"SELCTD ABUS ADPTR:"},
  {1,3,1,1,fc_72," "},
  {1,3,1,1,fc_73," "},
  {1,3,1,1,fc_74," "},
  {1,3,1,1,fc_75," "},
  {1,3,1,1,fc_76," "},
  {1,3,1,1,fc_77," "},
  {2,1,2,0,0,"LONGWORD CNT:"},
  {4,3,1,16,fc_78,"MBOX CYCLE:"},
  {2,3,1,4,fc_79,"MBOX DEST:"} };
DD$REGISTER_FIELD_DSD  rf_m86ms2[] = { 
  {1,7,1,0,0," "},
  {1,3,1,1,fc_80," "},
  {1,3,1,1,fc_81," "},
  {1,3,1,1,fc_82," "},
  {1,3,1,1,fc_83," "},
  {1,3,1,1,fc_84," "},
  {1,3,1,1,fc_85," "},
  {1,3,1,1,fc_86," "},
  {4,1,3,0,0,"DIAG ABUS CMD/MASK:"},
  {2,1,2,0,0,"DIAG ABUS LEN/STAT:"},
  {1,3,1,1,fc_87," "},
  {1,3,1,1,fc_88," "},
  {5,1,3,0,0,"PAMM CONFIG CODE:"},
  {11,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m86mdecc[] = { 
  {1,3,1,1,fc_89," "},
  {7,1,3,0,0,"DIAG CHK BIT INVRT:"},
  {1,7,1,0,0," "},
  {6,1,3,0,0,"ECC DATA CORR SYNDR:"},
  {1,3,1,1,fc_90," "},
  {3,1,3,0,0,"ECC2 DATA:"},
  {1,3,1,1,fc_91," "},
  {1,3,1,1,fc_92," "},
  {1,3,1,1,fc_93," "},
  {1,3,1,1,fc_94," "},
  {9,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m86cslint[] = { 
  {6,1,3,0,0,"CBUS ADDRESS:"},
  {1,3,1,2,fc_95," "},
  {1,3,1,2,fc_96," "},
  {8,1,3,0,0,"CBUS DATA:"},
  {4,1,3,0,0,"EBOX IPR STAT:"},
  {1,3,1,2,fc_97," "},
  {2,1,3,0,0,"HIEST IPR IOA:"},
  {1,3,1,1,fc_98," "},
  {1,3,1,1,fc_99," "},
  {1,3,1,1,fc_100," "},
  {1,3,1,1,fc_101," "},
  {1,3,1,1,fc_102," "},
  {1,3,1,1,fc_103," "},
  {1,3,1,1,fc_104," "},
  {2,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m86merg[] = { 
  {1,3,1,1,fc_105," "},
  {1,3,1,1,fc_106," "},
  {1,3,1,1,fc_107," "},
  {1,3,1,1,fc_108," "},
  {1,3,1,1,fc_109," "},
  {1,3,1,1,fc_110," "},
  {2,7,1,0,0," "},
  {1,3,1,1,fc_111," "},
  {1,3,1,1,fc_112," "},
  {1,3,1,1,fc_113," "},
  {1,3,1,1,fc_114," "},
  {1,3,1,1,fc_115," "},
  {19,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m86cshctl[] = { 
  {3,3,1,8,fc_116," "},
  {1,3,1,1,fc_117," "},
  {28,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m86fber[] = { 
  {1,3,1,1,fc_118," "},
  {1,7,1,0,0," "},
  {1,1,3,0,0,"FBR4 WBUS:"},
  {11,7,1,0,0," "},
  {2,3,1,4,fc_119,"FBOX INSTR FMT:"},
  {1,3,1,1,fc_120," "},
  {1,3,1,1,fc_121," "},
  {1,3,1,1,fc_122," "},
  {1,3,1,1,fc_123,"FBOX DRAM PE"},
  {1,3,1,1,fc_124," "},
  {1,3,1,1,fc_125," "},
  {2,7,1,0,0," "},
  {2,3,1,4,fc_126,"EXPONENT XTENSN: "},
  {6,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m86cses[] = { 
  {3,3,1,7,fc_127," "},
  {5,7,1,0,0," "},
  {8,1,3,0,0,"CS SYNDROME:"},
  {13,1,3,0,0,"CS/DRAM ADDRESS:"},
  {2,7,1,0,0," "},
  {1,3,1,1,fc_128," "} };
DD$REGISTER_FIELD_DSD  rf_psl[] = { 
  {4,1,3,0,0,"CONDITION CODE = "},
  {1,3,1,1,fc_129," "},
  {1,3,1,1,fc_130," "},
  {1,3,1,1,fc_131," "},
  {1,3,1,1,fc_132," "},
  {8,7,1,0,0," "},
  {5,1,3,0,0,"IPL"},
  {1,7,1,0,0," "},
  {2,3,1,4,fc_133,"PREV ACCS MODE: "},
  {2,3,1,4,fc_134,"CURR ACCS MODE: "},
  {1,3,1,1,fc_135," "},
  {1,3,1,1,fc_136," "},
  {2,7,1,0,0," "},
  {1,3,1,1,fc_137," "},
  {1,3,1,2,fc_138," "} };
DD$REGISTER_FIELD_DSD  rf_mcksumm[] = { 
  {32,3,1,14,fc_139," "} };
DD$REGISTER_FIELD_DSD  rf_mc8cpues[] = { 
  {1,7,1,0,0," "},
  {2,1,3,0,0,"AST LEVEL:"},
  {1,3,1,1,fc_140," "},
  {3,3,1,7,fc_141,"ARTH TRP CODE: "},
  {1,3,1,1,fc_142," "},
  {1,3,1,1,fc_143," "},
  {1,3,1,1,fc_144," "},
  {1,3,1,1,fc_145," "},
  {1,3,1,1,fc_146," "},
  {3,3,1,3,fc_147,"CONTRL STOR PE IN"},
  {1,3,1,1,fc_148," "},
  {1,3,1,1,fc_149," "},
  {15,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_mc8tb0[] = { 
  {1,3,1,1,fc_150," "},
  {4,3,1,12,fc_151,"FORCE TB PE ON"},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_152," "},
  {1,3,1,1,fc_153," "},
  {1,3,1,1,fc_154," "},
  {1,3,1,1,fc_155," "},
  {4,1,3,0,0,"UMCT STATUS:"},
  {1,1,3,0,0,"UADS STATUS:"},
  {1,1,3,0,0,"UFS STATUS:"},
  {1,3,1,1,fc_156," "},
  {1,3,1,1,fc_157," "},
  {1,3,1,1,fc_158," "},
  {1,3,1,1,fc_159," "},
  {1,3,1,1,fc_160," "},
  {11,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_mc8tb1[] = { 
  {1,3,1,1,fc_161," "},
  {1,3,1,1,fc_162," "},
  {1,3,1,1,fc_163," "},
  {1,3,1,1,fc_164," "},
  {1,3,1,1,fc_165," "},
  {1,7,1,0,0," "},
  {1,3,1,2,fc_166,"LST TB GRP WRTN:"},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_167," "},
  {12,1,3,0,0,"TB PE STATUS:"},
  {11,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_mc8sbier[] = { 
  {1,7,1,0,0," "},
  {1,3,1,1,fc_168," "},
  {1,3,1,1,fc_169," "},
  {1,3,1,1,fc_170," "},
  {3,3,1,3,fc_171," "},
  {1,3,1,1,fc_172," "},
  {1,3,1,1,fc_173," "},
  {1,7,1,0,0," "},
  {3,3,1,3,fc_174," "},
  {1,3,1,1,fc_175," "},
  {1,3,1,1,fc_176," "},
  {1,3,1,1,fc_177," "},
  {16,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_mc5sav[] = { 
  {2,3,1,4,fc_178,"CPU MODE: "},
  {1,3,1,2,fc_179," "},
  {1,3,1,2,fc_180," "},
  {28,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_mc5tbg[] = { 
  {1,3,1,1,fc_181," "},
  {1,3,1,1,fc_182," "},
  {1,3,1,2,fc_183," "},
  {1,3,1,2,fc_184," "},
  {28,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_mc5cache[] = { 
  {1,3,1,1,fc_185," "},
  {1,3,1,1,fc_186," "},
  {1,3,1,1,fc_187," "},
  {1,3,1,1,fc_188," "},
  {28,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_mc5bus[] = { 
  {1,3,1,1,fc_189," "},
  {1,3,1,1,fc_190," "},
  {1,3,1,1,fc_191," "},
  {1,3,1,1,fc_192," "},
  {28,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_mc5mce[] = { 
  {1,3,1,2,fc_193," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_194," "},
  {1,3,1,1,fc_195," "},
  {28,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m88mcs[] = { 
  {1,3,1,1,fc_196," "},
  {2,3,1,1,fc_197," "},
  {1,3,1,1,fc_198," "},
  {29,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m88iber[] = { 
  {1,3,1,1,fc_199," "},
  {1,3,1,1,fc_200," "},
  {1,3,1,1,fc_201," "},
  {1,3,1,1,fc_202," "},
  {1,3,1,1,fc_203," "},
  {1,3,1,1,fc_204," "},
  {1,3,1,1,fc_205," "},
  {1,3,1,1,fc_206," "},
  {1,3,1,1,fc_207," "},
  {1,3,1,1,fc_208," "},
  {1,3,1,1,fc_209," "},
  {1,3,1,1,fc_210," "},
  {20,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m88cber[] = { 
  {1,3,1,1,fc_211," "},
  {1,3,1,1,fc_212," "},
  {1,3,1,1,fc_213," "},
  {1,3,1,1,fc_214," "},
  {1,3,1,1,fc_215," "},
  {3,7,1,0,0," "},
  {4,3,1,17,fc_216," "},
  {3,3,1,7,fc_217," "},
  {1,7,1,0,0," "},
  {3,3,1,7,fc_218," "},
  {1,7,1,0,0," "},
  {2,3,1,3,fc_219," "},
  {10,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m88nmf[] = { 
  {16,7,1,0,0," "},
  {3,3,1,6,fc_220," "},
  {1,3,1,1,fc_221," "},
  {1,3,1,1,fc_222," "},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_223," "},
  {1,3,1,1,fc_224," "},
  {2,3,1,3,fc_225," "},
  {1,3,1,1,fc_226," "},
  {3,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m82stat[] = { 
  {1,3,1,1,fc_227," "},
  {1,3,1,1,fc_228," "},
  {1,3,1,1,fc_229," "},
  {1,3,1,1,fc_230," "},
  {1,3,1,1,fc_231," "},
  {7,7,1,0,0," "},
  {1,3,1,1,fc_232," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_233," "},
  {1,7,1,0,0," "},
  {5,3,1,13,fc_234,"VAXBI EVENT: "},
  {1,3,1,1,fc_235," "},
  {1,3,1,1,fc_236," "},
  {7,7,1,0,0," "},
  {1,3,1,1,fc_237," "},
  {1,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_awer[] = { 
  {1,7,1,0,0," "},
  {1,3,1,1,fc_238," "},
  {1,3,1,1,fc_239," "},
  {1,3,1,1,fc_240," "},
  {3,3,1,3,fc_241,"IB TIMEOUT:"},
  {1,3,1,1,fc_242," "},
  {1,3,1,1,fc_243," "},
  {1,7,1,0,0," "},
  {3,3,1,3,fc_244,"CP TIMEOUT: "},
  {1,3,1,1,fc_245," "},
  {1,3,1,1,fc_246," "},
  {1,3,1,1,fc_247," "},
  {16,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_awfs[] = { 
  {16,7,1,0,0," "},
  {1,3,1,1,fc_248," "},
  {1,3,1,1,fc_249," "},
  {1,3,1,1,fc_250," "},
  {1,3,1,1,fc_251," "},
  {6,7,1,0,0," "},
  {1,3,1,1,fc_252," "},
  {1,3,1,1,fc_253," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_254," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_255," "} };
DD$REGISTER_FIELD_DSD  rf_awsc[] = { 
  {16,7,1,0,0," "},
  {4,1,3,0,0,"COUNT FIELD:"},
  {3,1,3,0,0,"COMPARE TAG:"},
  {4,1,3,0,0,"COMPAR/CMD MASK:"},
  {2,1,3,0,0,"CONDIT LOCK CODE:"},
  {1,3,1,1,fc_256," "},
  {1,3,1,1,fc_257," "},
  {1,3,1,1,fc_258," "} };
DD$REGISTER_FIELD_DSD  rf_awmt[] = { 
  {8,7,1,0,0," "},
  {1,3,1,1,fc_259," "},
  {2,3,1,3,fc_260," "},
  {1,3,1,1,fc_261," "},
  {1,3,1,1,fc_262," "},
  {2,3,1,3,fc_263," "},
  {2,3,1,3,fc_264," "},
  {4,1,3,0,0,"REVRS CACHE PARITY:"},
  {1,3,1,1,fc_265," "},
  {1,3,1,1,fc_266," "},
  {5,1,2,0,0,"MAINT ID:"},
  {1,3,1,1,fc_267," "},
  {1,3,1,1,fc_268," "},
  {1,3,1,1,fc_269," "},
  {1,3,1,1,fc_270," "} };
DD$REGISTER_FIELD_DSD  rf_ioadc[] = { 
  {1,3,1,1,fc_271," "},
  {1,3,1,1,fc_272," "},
  {1,3,1,1,fc_273," "},
  {1,3,1,1,fc_274," "},
  {1,3,1,1,fc_275," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_276," "},
  {1,3,1,1,fc_277," "},
  {1,3,1,1,fc_278," "},
  {7,7,1,0,0," "},
  {1,3,1,1,fc_279," "},
  {1,3,1,1,fc_280," "},
  {1,3,1,1,fc_281," "},
  {1,3,1,1,fc_282," "},
  {12,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_ioaes[] = { 
  {1,3,1,1,fc_283," "},
  {1,3,1,1,fc_284," "},
  {1,3,1,1,fc_285," "},
  {1,3,1,1,fc_286," "},
  {1,3,1,1,fc_287," "},
  {1,3,1,1,fc_288," "},
  {1,3,1,1,fc_289," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_290," "},
  {1,3,1,1,fc_291," "},
  {1,3,1,1,fc_292," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_293," "},
  {1,3,1,1,fc_294," "},
  {1,3,1,1,fc_295," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_296," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_297," "},
  {1,3,1,1,fc_298," "},
  {1,3,1,1,fc_299," "},
  {1,3,1,1,fc_300," "},
  {1,3,1,1,fc_301," "},
  {1,3,1,1,fc_302," "},
  {2,7,1,0,0," "},
  {2,1,2,0,0,"ABUS LENGTH/STAT:"},
  {4,1,3,0,0,"ABUS CMD:"} };
DD$REGISTER_FIELD_DSD  rf_ioacs[] = { 
  {24,7,1,0,0," "},
  {4,1,3,0,0,"CPU TR SELECT:"},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_303," "},
  {1,3,1,1,fc_304," "},
  {1,3,1,1,fc_305," "} };
DD$REGISTER_FIELD_DSD  rf_ioacf[] = { 
  {4,1,2,0,0,"ABUS ADPTR REV:"},
  {4,1,3,0,0,"ABUS ADPTR TYPE:"},
  {12,7,1,0,0," "},
  {10,1,3,0,0,"MEMORY SEPARATOR:"},
  {2,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_sbiaer[] = { 
  {8,7,1,0,0," "},
  {1,3,1,1,fc_306," "},
  {1,7,1,0,0," "},
  {3,3,1,3,fc_307,"CP TIMEOUT:"},
  {19,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_sbiafs[] = { 
  {16,7,1,0,0," "},
  {1,3,1,1,fc_308," "},
  {1,3,1,1,fc_309," "},
  {1,3,1,1,fc_310," "},
  {1,3,1,1,fc_311," "},
  {2,7,1,0,0," "},
  {1,3,1,1,fc_312," "},
  {1,3,1,1,fc_313," "},
  {2,7,1,0,0," "},
  {1,3,1,1,fc_314," "},
  {1,3,1,1,fc_315," "},
  {1,3,1,1,fc_316," "},
  {1,3,1,1,fc_317," "},
  {1,3,1,1,fc_318," "},
  {1,3,1,1,fc_319," "} };
DD$REGISTER_FIELD_DSD  rf_sbiasc[] = { 
  {16,7,1,0,0," "},
  {4,1,2,0,0,"COUNT FIELD:"},
  {3,1,3,0,0,"COMPARE TAG:"},
  {4,1,3,0,0,"COMPAR/CMD MASK:"},
  {2,1,3,0,0,"CNDTL LOCK CODE:"},
  {1,3,1,1,fc_320," "},
  {1,3,1,1,fc_321," "},
  {1,3,1,1,fc_322," "} };
DD$REGISTER_FIELD_DSD  rf_sbiamr[] = { 
  {1,3,1,1,fc_323," "},
  {1,3,1,1,fc_324," "},
  {1,3,1,1,fc_325," "},
  {1,3,1,1,fc_326," "},
  {1,3,1,1,fc_327," "},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_328," "},
  {2,7,1,0,0," "},
  {1,3,1,1,fc_329," "},
  {11,7,1,0,0," "},
  {5,1,3,0,0,"MAINT ID:"},
  {1,3,1,1,fc_330," "},
  {1,3,1,1,fc_331," "},
  {1,3,1,1,fc_332," "},
  {1,3,1,1,fc_333," "} };
DD$REGISTER_FIELD_DSD  rf_ubacf[] = { 
  {8,1,3,0,0,"UBA CODE:"},
  {8,7,1,0,0," "},
  {1,3,1,1,fc_334," "},
  {1,3,1,1,fc_335," "},
  {1,3,1,1,fc_336," "},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_337," "},
  {1,3,1,1,fc_338," "},
  {2,7,1,0,0," "},
  {1,3,1,1,fc_339," "},
  {1,3,1,1,fc_340," "},
  {1,3,1,1,fc_341," "},
  {1,3,1,1,fc_342," "},
  {1,3,1,1,fc_343," "},
  {1,3,1,1,fc_344," "} };
DD$REGISTER_FIELD_DSD  rf_ubacr[] = { 
  {1,3,1,1,fc_345," "},
  {1,3,1,1,fc_346," "},
  {1,3,1,1,fc_347," "},
  {1,3,1,1,fc_348," "},
  {1,3,1,1,fc_349," "},
  {1,3,1,1,fc_350," "},
  {1,3,1,1,fc_351," "},
  {19,7,1,0,0," "},
  {5,1,3,0,0,"MAP REG DISA BITS:"},
  {1,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_ubasr[] = { 
  {1,3,1,1,fc_352," "},
  {1,3,1,1,fc_353," "},
  {1,3,1,1,fc_354," "},
  {1,3,1,1,fc_355," "},
  {1,3,1,1,fc_356," "},
  {1,3,1,1,fc_357," "},
  {1,3,1,1,fc_358," "},
  {1,3,1,1,fc_359," "},
  {1,3,1,1,fc_360," "},
  {1,3,1,1,fc_361," "},
  {1,3,1,1,fc_362," "},
  {13,7,1,0,0," "},
  {1,3,1,1,fc_363," "},
  {1,3,1,1,fc_364," "},
  {1,3,1,1,fc_365," "},
  {1,3,1,1,fc_366," "},
  {4,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_ubadcr[] = { 
  {8,1,3,0,0,"UBA CODE:"},
  {8,7,1,0,0," "},
  {1,3,1,1,fc_367," "},
  {1,3,1,1,fc_368," "},
  {1,3,1,1,fc_369," "},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_370," "},
  {1,3,1,1,fc_371," "},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_372," "},
  {1,3,1,1,fc_373," "},
  {1,3,1,1,fc_374," "},
  {1,3,1,1,fc_375," "},
  {1,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_memer0[] = { 
  {3,3,1,5,fc_376,"INTERLEAVE:"},
  {2,3,1,4,fc_377,"RAM:"},
  {3,1,3,0,0,"ADAPTER CODE:"},
  {1,7,1,0,0," "},
  {6,1,2,0,0,"MEMORY SIZE:"},
  {1,3,1,1,fc_378," "},
  {1,3,1,1,fc_379," "},
  {1,3,1,1,fc_380," "},
  {1,3,1,1,fc_381," "},
  {1,3,1,1,fc_382," "},
  {1,3,1,1,fc_383," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_384," "},
  {1,3,1,1,fc_385," "},
  {2,7,1,0,0," "},
  {1,3,1,1,fc_386," "},
  {1,3,1,1,fc_387," "},
  {1,3,1,1,fc_388," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_389," "},
  {1,3,1,1,fc_390," "} };
DD$REGISTER_FIELD_DSD  rf_memer1[] = { 
  {7,1,3,0,0,"DIAG ECC BITS:"},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_391," "},
  {2,3,1,3,fc_392,"DIAG MODE:"},
  {1,3,1,1,fc_393," "},
  {2,3,1,3,fc_394," "},
  {1,3,1,1,fc_395," "},
  {4,7,1,0,0," "},
  {9,1,3,0,0,"START ADDR:"},
  {4,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_memer2[] = { 
  {7,1,3,0,0,"ERR SYNDR/CHK BITS:"},
  {1,3,1,1,fc_396," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_397," "},
  {1,3,1,1,fc_398," "},
  {17,1,3,0,0,"ERROR ADDR:"},
  {1,3,1,1,fc_399," "},
  {1,3,1,1,fc_400," "},
  {1,3,1,1,fc_401," "},
  {1,3,1,1,fc_402," "} };
DD$REGISTER_FIELD_DSD  rf_memer3[] = { 
  {7,1,3,0,0,"ERR SYNDR/CHK BITS:"},
  {1,3,1,1,fc_403," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_404," "},
  {1,3,1,1,fc_405," "},
  {17,1,3,0,0,"ERROR ADDR:"},
  {1,3,1,1,fc_406," "},
  {1,3,1,1,fc_407," "},
  {1,3,1,1,fc_408," "},
  {1,3,1,1,fc_409," "} };
DD$REGISTER_FIELD_DSD  rf_bidevreg[] = { 
  {16,1,3,0,0,"DEVICE TYPE:"},
  {16,1,2,0,0,"DEVICE REV:"} };
DD$REGISTER_FIELD_DSD  rf_bicsr[] = { 
  {4,1,2,0,0,"NODE ID:"},
  {2,3,1,4,fc_410,"ARBITR CNTRL"},
  {1,3,1,1,fc_411," "},
  {1,3,1,1,fc_412," "},
  {1,3,1,1,fc_413," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_414," "},
  {1,3,1,1,fc_415," "},
  {1,3,1,1,fc_416," "},
  {1,3,1,1,fc_417," "},
  {1,3,1,1,fc_418," "},
  {1,3,1,1,fc_419," "},
  {8,1,3,0,0,"BI INTERFACE DEV:"},
  {8,1,2,0,0,"BI DEVICE REV:"} };
DD$REGISTER_FIELD_DSD  rf_bibuserreg[] = { 
  {1,3,1,1,fc_420," "},
  {1,3,1,1,fc_421," "},
  {1,3,1,1,fc_422," "},
  {1,3,1,1,fc_423," "},
  {12,7,1,0,0," "},
  {1,3,1,1,fc_424," "},
  {1,3,1,1,fc_425," "},
  {1,3,1,1,fc_426," "},
  {1,3,1,1,fc_427," "},
  {1,3,1,1,fc_428,"RETRY TIMEOUT"},
  {1,3,1,1,fc_429," "},
  {1,3,1,1,fc_430," "},
  {1,3,1,1,fc_431," "},
  {1,3,1,1,fc_432," "},
  {1,3,1,1,fc_433," "},
  {1,3,1,1,fc_434," "},
  {1,3,1,1,fc_435," "},
  {1,3,1,1,fc_436," "},
  {1,3,1,1,fc_437," "},
  {1,3,1,1,fc_438," "},
  {1,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_bierint[] = { 
  {2,7,1,0,0," "},
  {12,1,3,0,0,"ERROR INTERRUPT VECTOR:"},
  {2,7,1,0,0," "},
  {4,1,2,0,0,"INTRPT LEVEL:"},
  {1,3,1,1,fc_439," "},
  {1,3,1,1,fc_440," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_441," "},
  {1,3,1,1,fc_442," "},
  {7,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_biintdst[] = { 
  {16,1,2,0,0,"INTRPT DEST:"},
  {16,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_buacsr[] = { 
  {8,3,1,18,fc_443,"FAILED SELF TEST - "},
  {8,7,1,0,0," "},
  {1,3,1,1,fc_444," "},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_445," "},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_446," "},
  {1,3,1,1,fc_447," "},
  {1,3,1,1,fc_448," "},
  {1,3,1,1,fc_449," "},
  {1,3,1,1,fc_450," "},
  {2,7,1,0,0," "},
  {1,3,1,1,fc_451," "} };
DD$REGISTER_FIELD_DSD  rf_dsaformat[] = { 
  {16,3,1,10,fc_452," "} };
DD$REGISTER_FIELD_DSD  rf_dsaflags[] = { 
  {1,3,1,1,fc_453," "},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_454," "},
  {1,3,1,1,fc_455," "},
  {1,3,1,1,fc_456," "},
  {1,3,1,1,fc_457," "},
  {8,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_event_code[] = { 
  {5,3,1,17,fc_458," "},
  {11,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_dsastat[] = { 
  {16,3,1,83,fc_459," "} };
DD$REGISTER_FIELD_DSD  rf_cntrlid[] = { 
  {8,3,1,20,fc_460," "},
  {8,3,1,1,fc_461," "} };
DD$REGISTER_FIELD_DSD  rf_unitid2[] = { 
  {8,3,1,17,fc_462," "},
  {6,3,1,3,fc_463," "} };
DD$REGISTER_FIELD_DSD  rf_std_retlvl[] = { 
  {4,1,3,0,0,"LEVEL:"},
  {4,1,2,0,0,"RETRY COUNT:"} };
DD$REGISTER_FIELD_DSD  rf_replflg[] = { 
  {10,7,1,0,0," "},
  {1,3,1,1,fc_464," "},
  {1,3,1,1,fc_465," "},
  {1,3,1,1,fc_466," "},
  {1,3,1,1,fc_467," "},
  {1,3,1,1,fc_468," "},
  {1,3,1,1,fc_469," "} };
DD$REGISTER_FIELD_DSD  rf_sd1stat[] = { 
  {1,3,1,2,fc_470," "},
  {1,3,1,2,fc_471," "},
  {1,7,1,0,0," "},
  {1,3,1,2,fc_472," "},
  {1,3,1,2,fc_473," "},
  {1,3,1,1,fc_474," "},
  {1,3,1,2,fc_475," "},
  {1,3,1,2,fc_476," "},
  {1,3,1,2,fc_477," "},
  {1,3,1,1,fc_478," "},
  {1,3,1,2,fc_479," "},
  {1,3,1,2,fc_480," "},
  {1,3,1,2,fc_481," "},
  {6,7,1,0,0," "},
  {1,3,1,1,fc_482," "},
  {1,3,1,1,fc_483," "},
  {1,3,1,1,fc_484," "},
  {1,3,1,1,fc_485," "},
  {1,3,1,1,fc_486," "},
  {4,3,1,3,fc_487," "},
  {4,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_sd2stat[] = { 
  {8,1,2,0,0,"RETRY CNT/FAILURE CODE: "},
  {16,1,2,0,0,"PREVIOUS CYLINDER"},
  {8,1,2,0,0,"PREVIOUS GROUP"} };
DD$REGISTER_FIELD_DSD  rf_sd3stat[] = { 
  {16,1,2,0,0,"CURRENT CYLINDER:"},
  {8,1,2,0,0,"CURRENT GROUP:"},
  {8,1,3,0,0,"FRT PANEL FLT CODE:"} };
DD$REGISTER_FIELD_DSD  rf_sdireqbyte[] = { 
  {1,3,1,2,fc_488," "},
  {1,3,1,2,fc_489," "},
  {1,3,1,2,fc_490," "},
  {1,3,1,2,fc_491," "},
  {1,3,1,2,fc_492," "},
  {1,3,1,1,fc_493," "},
  {1,3,1,1,fc_494," "},
  {1,3,1,2,fc_495," "} };
DD$REGISTER_FIELD_DSD  rf_sdimodbyte[] = { 
  {1,3,1,2,fc_496," "},
  {1,3,1,2,fc_497," "},
  {1,3,1,2,fc_498," "},
  {1,3,1,2,fc_499," "},
  {1,3,1,2,fc_500," "},
  {3,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_sdierrbyte[] = { 
  {3,7,1,0,0," "},
  {1,3,1,1,fc_501," "},
  {1,3,1,1,fc_502," "},
  {1,3,1,1,fc_503," "},
  {1,3,1,1,fc_504," "},
  {1,3,1,1,fc_505," "} };
DD$REGISTER_FIELD_DSD  rf_sdictlbyte[] = { 
  {4,3,1,2,fc_506," "} };
DD$REGISTER_FIELD_DSD  rf_sdilstop[] = { 
  {8,3,1,16,fc_507," "} };
DD$REGISTER_FIELD_DSD  rf_sdidrvdet[] = { 
  {3,7,1,0,0," "},
  {1,3,1,1,fc_508," "},
  {1,3,1,1,fc_509," "},
  {1,3,1,1,fc_510," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_511," "} };
DD$REGISTER_FIELD_DSD  rf_tp_event_code[] = { 
  {5,3,1,19,fc_512," "},
  {11,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_tp_dsastat[] = { 
  {16,3,1,19,fc_513," "} };
DD$REGISTER_FIELD_DSD  rf_tp_unitid2[] = { 
  {16,7,1,0,0," "},
  {8,3,1,3,fc_514," "},
  {8,3,1,3,fc_515," "} };
DD$REGISTER_FIELD_DSD  rf_st1drvst[] = { 
  {8,1,2,0,0,"DRIVE SPEED:"},
  {8,1,2,0,0,"TAPE DENSITY:"},
  {16,1,2,0,0,"MSCP UNIT:"} };
DD$REGISTER_FIELD_DSD  rf_stisumm[] = { 
  {1,3,1,1,fc_516," "},
  {1,3,1,1,fc_517," "},
  {1,3,1,2,fc_518," "},
  {1,3,1,1,fc_519," "},
  {1,3,1,1,fc_520," "},
  {1,3,1,1,fc_521," "},
  {1,3,1,1,fc_522," "},
  {1,3,1,1,fc_523," "},
  {4,7,1,0,0," "},
  {1,3,1,1,fc_524," "},
  {1,3,1,1,fc_525," "},
  {1,3,1,1,fc_526," "},
  {1,3,1,1,fc_527," "},
  {1,3,1,1,fc_528," "},
  {1,3,1,1,fc_529," "},
  {1,3,1,1,fc_530," "},
  {1,3,1,1,fc_531," "},
  {4,7,1,0,0," "},
  {8,1,2,0,0,"CONTROLLER BYTE: "} };
DD$REGISTER_FIELD_DSD  rf_stidrv0[] = { 
  {1,3,1,1,fc_532," "},
  {1,3,1,1,fc_533," "},
  {1,3,1,1,fc_534," "},
  {1,3,1,1,fc_535," "},
  {1,3,1,1,fc_536," "},
  {1,3,1,1,fc_537," "},
  {1,3,1,1,fc_538," "},
  {1,3,1,1,fc_539," "},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_540," "},
  {1,3,1,1,fc_541," "},
  {1,3,1,1,fc_542," "},
  {1,3,1,1,fc_543," "},
  {1,3,1,1,fc_544," "} };
DD$REGISTER_FIELD_DSD  rf_stidrv1[] = { 
  {1,3,1,1,fc_545," "},
  {1,3,1,1,fc_546," "},
  {1,3,1,1,fc_547," "},
  {1,3,1,1,fc_548," "},
  {1,3,1,1,fc_549," "},
  {1,3,1,1,fc_550," "},
  {1,3,1,1,fc_551," "},
  {1,3,1,1,fc_552," "},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_553," "},
  {1,3,1,1,fc_554," "},
  {1,3,1,1,fc_555," "},
  {1,3,1,1,fc_556," "},
  {1,3,1,1,fc_557," "} };
DD$REGISTER_FIELD_DSD  rf_stidrv2[] = { 
  {1,3,1,1,fc_558," "},
  {1,3,1,1,fc_559," "},
  {1,3,1,1,fc_560," "},
  {1,3,1,1,fc_561," "},
  {1,3,1,1,fc_562," "},
  {1,3,1,1,fc_563," "},
  {1,3,1,1,fc_564," "},
  {1,3,1,1,fc_565," "},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_566," "},
  {1,3,1,1,fc_567," "},
  {1,3,1,1,fc_568," "},
  {1,3,1,1,fc_569," "},
  {1,3,1,1,fc_570," "} };
DD$REGISTER_FIELD_DSD  rf_stidrv3[] = { 
  {1,3,1,1,fc_571," "},
  {1,3,1,1,fc_572," "},
  {1,3,1,1,fc_573," "},
  {1,3,1,1,fc_574," "},
  {1,3,1,1,fc_575," "},
  {1,3,1,1,fc_576," "},
  {1,3,1,1,fc_577," "},
  {1,3,1,1,fc_578," "},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_579," "},
  {1,3,1,1,fc_580," "},
  {1,3,1,1,fc_581," "},
  {1,3,1,1,fc_582," "},
  {1,3,1,1,fc_583," "} };
DD$REGISTER_FIELD_DSD  rf_m82summ[] = { 
  {1,3,1,1,fc_584," "},
  {1,3,1,1,fc_585," "},
  {1,3,1,1,fc_586," "},
  {1,3,1,1,fc_587," "},
  {1,3,1,1,fc_588," "},
  {1,3,1,1,fc_589," "},
  {1,3,1,1,fc_590," "},
  {1,3,1,1,fc_591," "},
  {24,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m8cmera[] = { 
  {3,3,1,2,fc_592,"INTERLEAVE:"},
  {2,3,1,4,fc_593,"MEM TYPE:"},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_594," "},
  {6,3,1,16,fc_595,"ARRAY SIZE:"},
  {7,7,1,0,0," "},
  {1,3,1,1,fc_596," "},
  {1,3,1,1,fc_597," "},
  {2,7,1,0,0," "},
  {1,3,1,1,fc_598," "},
  {1,3,1,1,fc_599," "},
  {1,3,1,1,fc_600," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_601," "},
  {1,3,1,1,fc_602," "} };
DD$REGISTER_FIELD_DSD  rf_mc8merb[] = { 
  {8,1,3,0,0,"ECC SUBST:"},
  {1,3,1,1,fc_603," "},
  {1,3,1,1,fc_604," "},
  {2,7,1,0,0," "},
  {2,3,1,3,fc_605," "},
  {1,3,1,1,fc_606," "},
  {13,1,3,0,0,"CNTR START ADDR:"},
  {2,1,3,0,0,"FILE INP PTR:"},
  {2,1,3,0,0,"FILE OUTP PTR:"} };
DD$REGISTER_FIELD_DSD  rf_m8cmerc[] = { 
  {8,1,3,0,0,"ERR SYNDR:"},
  {20,1,3,0,0,"ERR ADDR:"},
  {1,3,1,1,fc_607," "},
  {1,3,1,1,fc_608," "},
  {1,3,1,1,fc_609," "},
  {1,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m5mer0[] = { 
  {7,1,3,0,0,"ERR SYNDR:"},
  {2,7,1,0,0," "},
  {15,1,3,0,0,"ERR PAGE ADDR:"},
  {5,7,1,0,0," "},
  {1,3,1,1,fc_610," "},
  {1,3,1,1,fc_611," "},
  {1,3,1,1,fc_612," "} };
DD$REGISTER_FIELD_DSD  rf_m5mer1[] = { 
  {7,1,3,0,0,"CHK BITS:"},
  {2,7,1,0,0," "},
  {15,1,3,0,0,"PAGE MODE ADDR:"},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_613," "},
  {1,3,1,1,fc_614," "},
  {1,3,1,1,fc_615," "},
  {1,3,1,1,fc_616," "},
  {3,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m5mer2[] = { 
  {16,1,2,0,0,"MEM PRESENT:"},
  {1,3,1,1,fc_617," "},
  {7,1,3,0,0,"START ADDR:"},
  {1,3,1,1,fc_618," "},
  {7,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_m88csr0[] = { 
  {8,1,3,0,0,"ADPTR COD:"},
  {8,7,1,0,0," "},
  {4,1,3,0,0,"MCL REV:"},
  {4,7,1,0,0," "},
  {3,3,1,1,fc_619," "},
  {1,3,1,1,fc_620," "},
  {1,3,1,1,fc_621," "},
  {1,3,1,1,fc_622," "},
  {1,3,1,1,fc_623," "},
  {1,3,1,1,fc_624," "} };
DD$REGISTER_FIELD_DSD  rf_m88csr1[] = { 
  {3,1,3,0,0,"PRIM RAM BRD:"},
  {1,3,1,1,fc_625," "},
  {1,7,1,0,0," "},
  {3,1,3,0,0,"ALT RAM BRD:"},
  {1,3,1,1,fc_626," "},
  {11,7,1,0,0," "},
  {1,3,1,1,fc_627," "},
  {1,7,1,0,0," "},
  {7,1,3,0,0,"DRAM ADDR:"},
  {1,7,1,0,0," "},
  {2,1,3,0,0,"WRD CNT IN ERR:"} };
DD$REGISTER_FIELD_DSD  rf_m88csr2[] = { 
  {7,1,3,0,0,"ERR SYNDR:"},
  {1,7,1,0,0," "},
  {7,1,3,0,0,"CHK BITS:"},
  {1,3,1,1,fc_628," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_629," "},
  {1,3,1,1,fc_630," "},
  {2,3,1,2,fc_631," "},
  {1,3,1,1,fc_632," "},
  {1,3,1,1,fc_633," "},
  {1,3,1,1,fc_634," "},
  {4,7,1,0,0," "},
  {1,3,1,1,fc_635," "},
  {1,3,1,1,fc_636," "},
  {1,3,1,1,fc_637," "},
  {1,3,1,1,fc_638," "} };
DD$REGISTER_FIELD_DSD  rf_m88csr3[] = { 
  {3,3,1,6,fc_639,"BRD 0 SIZE:"},
  {3,3,1,6,fc_640,"BRD 1 SIZE:"},
  {3,3,1,6,fc_641,"BRD 2 SIZE:"},
  {3,3,1,6,fc_642,"BRD 3 SIZE:"},
  {3,3,1,6,fc_643,"BRD 4 SIZE:"},
  {3,3,1,6,fc_644,"BRD 5 SIZE"},
  {3,3,1,6,fc_645,"BRD 6 SIZE:"},
  {3,3,1,6,fc_646,"BRD 7 SIZE:"},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_647," "},
  {1,3,1,1,fc_648," "},
  {1,3,1,1,fc_649," "},
  {1,3,1,1,fc_650," "},
  {1,3,1,1,fc_651," "},
  {1,3,1,1,fc_652," "},
  {1,3,1,1,fc_653," "} };
DD$REGISTER_FIELD_DSD  rf_m82csr0[] = { 
  {7,1,3,0,0,"DIAGNOSTIC C BITS:"},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_654," "},
  {1,3,1,1,fc_655," "},
  {1,3,1,1,fc_656," "},
  {1,3,1,1,fc_657," "},
  {1,3,1,1,fc_658," "},
  {1,3,1,1,fc_659," "},
  {1,3,1,1,fc_660," "},
  {2,1,3,0,0,"RAM TYPE:"},
  {11,1,2,0,0,"MEMORY SIZE:"},
  {1,3,1,1,fc_661," "},
  {1,3,1,1,fc_662," "},
  {1,3,1,1,fc_663," "} };
DD$REGISTER_FIELD_DSD  rf_m82csr1[] = { 
  {7,1,3,0,0,"ERR SYNDR:"},
  {2,7,1,0,0," "},
  {15,1,3,0,0,"ERR ADDR:"},
  {4,7,1,0,0," "},
  {1,3,1,1,fc_664," "},
  {1,3,1,1,fc_665," "},
  {1,3,1,1,fc_666," "},
  {1,3,1,1,fc_667," "} };
DD$REGISTER_FIELD_DSD  rf_blacsr[] = { 
  {8,3,1,41,fc_668,"FAILD SLF TST:"},
  {8,7,1,0,0," "},
  {1,3,1,1,fc_669," "},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_670," "},
  {4,7,1,0,0," "},
  {1,3,1,1,fc_671," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_672," "},
  {1,3,1,1,fc_673," "},
  {1,3,1,1,fc_674," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_675," "} };
DD$REGISTER_FIELD_DSD  rf_cierrcod[] = { 
  {32,3,1,67,fc_676," "} };
DD$REGISTER_FIELD_DSD  rf_cipcnf[] = { 
  {8,1,3,0,0,"ADAPTER CODE:"},
  {1,3,1,1,fc_677," "},
  {1,3,1,1,fc_678," "},
  {1,3,1,1,fc_679," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_680," "},
  {1,3,1,1,fc_681," "},
  {1,3,1,1,fc_682," "},
  {1,3,1,1,fc_683," "},
  {1,3,1,1,fc_684," "},
  {1,3,1,1,fc_685," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_686," "},
  {1,3,1,1,fc_687," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_688," "},
  {1,3,1,1,fc_689," "},
  {8,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_cipmcsr[] = { 
  {1,3,1,1,fc_690," "},
  {1,3,1,1,fc_691," "},
  {1,3,1,1,fc_692," "},
  {1,3,1,1,fc_693," "},
  {1,3,1,1,fc_694," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_695," "},
  {1,3,1,1,fc_696," "},
  {1,3,1,1,fc_697," "},
  {1,3,1,1,fc_698," "},
  {1,3,1,1,fc_699," "},
  {1,3,1,1,fc_700," "},
  {1,3,1,1,fc_701," "},
  {1,3,1,1,fc_702," "},
  {1,3,1,1,fc_703," "},
  {1,3,1,1,fc_704," "},
  {16,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_cipsr[] = { 
  {1,3,1,1,fc_705," "},
  {1,3,1,1,fc_706," "},
  {1,3,1,1,fc_707," "},
  {1,3,1,1,fc_708," "},
  {1,3,1,1,fc_709," "},
  {1,3,1,1,fc_710," "},
  {1,3,1,1,fc_711," "},
  {24,7,1,0,0," "},
  {1,3,1,1,fc_712," "} };
DD$REGISTER_FIELD_DSD  rf_cippr[] = { 
  {8,1,2,0,0,"PORT NUMBER:"},
  {8,7,1,0,0," "},
  {12,1,2,0,0,"IBUF LENGTH:"},
  {3,7,1,0,0," "},
  {1,3,1,2,fc_713,"MAX NODES:"} };
DD$REGISTER_FIELD_DSD  rf_bicsr1[] = { 
  {4,1,2,0,0,"NODE ID:"},
  {2,3,1,4,fc_714,"ARBITR CNTRL:"},
  {1,3,1,1,fc_715," "},
  {1,3,1,1,fc_716," "},
  {1,3,1,1,fc_717," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_718," "},
  {1,3,1,1,fc_719," "},
  {1,3,1,1,fc_720," "},
  {1,3,1,1,fc_721," "},
  {1,3,1,1,fc_722," "},
  {1,3,1,1,fc_723," "},
  {8,1,3,0,0,"BI INTRFC DEV:"},
  {8,1,2,0,0,"INTRFC DEV REV:"} };
DD$REGISTER_FIELD_DSD  rf_nbiacsr0[] = { 
  {8,1,3,0,0,"NBI ADPTR CODE:"},
  {1,7,1,0,0," "},
  {5,1,2,0,0,"NBI VEC OFFSET:"},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_724," "},
  {1,3,1,1,fc_725," "},
  {1,3,1,1,fc_726," "},
  {1,3,1,1,fc_727," "},
  {1,3,1,1,fc_728," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_729," "},
  {2,7,1,0,0," "},
  {3,3,1,5,fc_730," "},
  {1,3,1,1,fc_731," "},
  {1,3,1,1,fc_732," "},
  {1,3,1,1,fc_733," "},
  {1,3,1,1,fc_734," "},
  {1,3,1,1,fc_735," "} };
DD$REGISTER_FIELD_DSD  rf_nbia1csr0[] = { 
  {8,1,3,0,0,"NBI ADPTR CODE:"},
  {1,7,1,0,0," "},
  {5,1,2,0,0,"NBI VEC OFFSET:"},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_736," "},
  {1,3,1,1,fc_737," "},
  {1,3,1,1,fc_738," "},
  {1,3,1,1,fc_739," "},
  {1,3,1,1,fc_740," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_741," "},
  {2,7,1,0,0," "},
  {3,3,1,5,fc_742," "},
  {1,3,1,1,fc_743," "},
  {1,3,1,1,fc_744," "},
  {1,3,1,1,fc_745," "},
  {1,3,1,1,fc_746," "},
  {1,3,1,1,fc_747," "} };
DD$REGISTER_FIELD_DSD  rf_nbiacsr1[] = { 
  {1,7,1,0,0," "},
  {1,3,1,1,fc_748," "},
  {1,3,1,1,fc_749," "},
  {1,3,1,1,fc_750," "},
  {1,3,1,1,fc_751," "},
  {1,3,1,1,fc_752," "},
  {1,3,1,1,fc_753," "},
  {1,3,1,1,fc_754," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_755," "},
  {1,3,1,1,fc_756," "},
  {1,7,1,0,0," "},
  {4,1,2,0,0,"NBIA MODULE REV:"},
  {16,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_bi1buserreg[] = { 
  {1,3,1,1,fc_757," "},
  {1,3,1,1,fc_758," "},
  {1,3,1,1,fc_759," "},
  {1,3,1,1,fc_760," "},
  {12,7,1,0,0," "},
  {1,3,1,1,fc_761," "},
  {1,3,1,1,fc_762," "},
  {1,3,1,1,fc_763," "},
  {1,3,1,1,fc_764," "},
  {1,3,1,1,fc_765,"RETRY TIMEOUT"},
  {1,3,1,1,fc_766," "},
  {1,3,1,1,fc_767," "},
  {1,3,1,1,fc_768," "},
  {1,3,1,1,fc_769," "},
  {1,3,1,1,fc_770," "},
  {1,3,1,1,fc_771," "},
  {1,3,1,1,fc_772," "},
  {1,3,1,1,fc_773," "},
  {1,3,1,1,fc_774," "},
  {1,3,1,1,fc_775," "},
  {1,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_ciper[] = { 
  {32,3,1,14,fc_776," "} };
DD$REGISTER_FIELD_DSD  rf_bvpsts[] = { 
  {6,7,1,0,0," "},
  {1,3,1,1,fc_777," "},
  {1,3,1,1,fc_778," "},
  {8,3,1,7,fc_779,"ERROR TYPE:"},
  {3,3,1,5,fc_780,"PORT STATE:"},
  {6,7,1,0,0," "},
  {1,3,1,1,fc_781," "},
  {1,3,1,1,fc_782," "},
  {1,3,1,1,fc_783," "},
  {1,3,1,1,fc_784," "},
  {1,3,1,1,fc_785," "},
  {1,3,1,1,fc_786," "},
  {1,3,1,2,fc_787," "} };
DD$REGISTER_FIELD_DSD  rf_bvperr[] = { 
  {24,3,1,23,fc_788," "},
  {8,1,2,0,0,"PORT #"} };
DD$REGISTER_FIELD_DSD  rf_cibcipcnf[] = { 
  {16,7,1,0,0," "},
  {4,3,1,6,fc_789,"RECV CMD:"},
  {3,7,1,0,0," "},
  {1,3,1,1,fc_790," "},
  {1,3,1,1,fc_791," "},
  {1,3,1,1,fc_792," "},
  {1,3,1,1,fc_793," "},
  {1,3,1,1,fc_794," "},
  {1,3,1,1,fc_795," "},
  {1,3,1,1,fc_796," "},
  {1,3,1,1,fc_797," "},
  {1,3,1,1,fc_798," "} };
DD$REGISTER_FIELD_DSD  rf_cibcapmcsr[] = { 
  {1,3,1,1,fc_799," "},
  {1,3,1,1,fc_800," "},
  {2,7,1,0,0," "},
  {1,3,1,1,fc_801," "},
  {1,3,1,1,fc_802," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_803," "},
  {1,3,1,1,fc_804," "},
  {1,3,1,1,fc_805," "},
  {1,3,1,1,fc_806," "},
  {1,3,1,1,fc_807," "},
  {1,3,1,1,fc_808," "},
  {1,3,1,1,fc_809,"INTERNAL BUS PARITY ERROR"},
  {1,3,1,1,fc_810," "},
  {1,3,1,1,fc_811," "},
  {16,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_cibcapsr[] = { 
  {1,3,1,1,fc_812," "},
  {1,3,1,1,fc_813," "},
  {1,3,1,1,fc_814," "},
  {1,3,1,1,fc_815," "},
  {1,3,1,1,fc_816," "},
  {1,3,1,1,fc_817," "},
  {1,3,1,1,fc_818," "},
  {1,3,1,1,fc_819," "},
  {1,3,1,1,fc_820," "},
  {1,3,1,1,fc_821," "},
  {1,3,1,1,fc_822," "},
  {4,7,1,0,0," "},
  {1,3,1,1,fc_823," "},
  {16,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_cibcappr[] = { 
  {8,1,2,0,0,"PORT NO:"},
  {2,3,1,4,fc_824," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_825," "},
  {1,3,1,1,fc_826," "},
  {3,7,1,0,0," "},
  {13,1,2,0,0,"IBUF LENGTH:"},
  {3,3,1,5,fc_827,"MAX NODES:"} };
DD$REGISTER_FIELD_DSD  rf_bvpcntl[] = { 
  {7,3,1,11,fc_828,"INSTRCTN"},
  {1,3,1,2,fc_829,"OWNER:"},
  {24,1,3,0,0,"DATA"} };
DD$REGISTER_FIELD_DSD  rf_mvaxmser[] = { 
  {1,3,1,1,fc_830," "},
  {1,3,1,1,fc_831," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_832," "},
  {1,3,1,1,fc_833," "},
  {1,3,1,1,fc_834," "},
  {1,3,1,1,fc_835," "},
  {1,3,1,1,fc_836," "},
  {2,3,1,4,fc_837,"PARITY ERR SOURCE"},
  {22,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_sxerr_csr[] = { 
  {8,7,1,0,0," "},
  {8,1,2,0,0,"MEMORY SIZE = "},
  {8,7,1,0,0," "},
  {1,3,1,2,fc_838," "},
  {1,3,1,1,fc_839," "},
  {1,3,1,1,fc_840," "},
  {1,3,1,1,fc_841," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_842," "},
  {1,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_vxstrmser[] = { 
  {1,3,1,1,fc_843," "},
  {1,3,1,1,fc_844," "},
  {4,7,1,0,0," "},
  {1,3,1,1,fc_845," "},
  {1,7,1,0,0," "},
  {1,3,1,1,fc_846," "},
  {23,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_uda5sa[] = { 
  {16,3,1,40,fc_847," "},
  {16,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_kdb5sa[] = { 
  {16,3,1,28,fc_848," "},
  {16,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_rqdx3sa[] = { 
  {16,3,1,32,fc_849," "},
  {16,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_mcuv1summ[] = { 
  {32,3,1,10,fc_850," "} };
DD$REGISTER_FIELD_DSD  rf_mcuv2summ[] = { 
  {32,3,1,13,fc_851," "} };
DD$REGISTER_FIELD_DSD  rf_mc73summ[] = { 
  {32,3,1,12,fc_852," "} };
DD$REGISTER_FIELD_DSD  rf_sdirtpi[] = { 
  {1,3,1,1,fc_853," "},
  {1,3,1,1,fc_854," "},
  {1,3,1,1,fc_855," "},
  {1,3,1,1,fc_856," "},
  {1,3,1,1,fc_857," "},
  {1,3,1,1,fc_858," "},
  {1,3,1,1,fc_859," "},
  {1,3,1,1,fc_860," "},
  {8,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_sdidrvst[] = { 
  {1,3,1,1,fc_861," "},
  {1,3,1,1,fc_862," "},
  {1,3,1,1,fc_863," "},
  {1,1,2,0,0,"FORMAT"},
  {1,3,1,1,fc_864," "},
  {1,3,1,1,fc_865," "},
  {1,3,1,1,fc_866," "},
  {8,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_tk57sa[] = { 
  {16,3,1,55,fc_867," "},
  {16,7,1,0,0," "} };
DD$REGISTER_FIELD_DSD  rf_rc25sa[] = { 
  {16,3,1,62,fc_868," "} };

/****************** STD_ITEM_STRUCT ******************/

DD$STD_ITEM_DSD  std_item_dsd_struct[] = { 
  {"eventtype",21,7,2,"OS EVENT TYPE",1,0,rf_eventtype},
  {"recordnumber",22,2,2,"SEQUENCE NUMBER",0,0,0},
  {"ostype",23,4,1,"OPERATING SYSTEM",4,sc_ostype,0},
  {"datetime",24,8,5,"OCCURRED/LOGGED ON",0,0,0},
  {"uptime",25,8,6,"SYSTEM UPTIME",0,0,0},
  {"serialnumber",26,2,3,"SYSTEM ID",0,0,0},
  {"eventclass",27,4,1,"EVENT CLASS",5,sc_eventclass,0},
  {"hostname",28,3,1,"OCCURRED ON SYSTEM",0,0,0},
  {"devclass",29,4,1,"UNIT CLASS",20,sc_devclass,0},
  {"devtype",30,4,1,"UNIT TYPE",156,sc_devtype,0},
  {"coarsesyndrome",31,5,1,"ERROR SYNDROME",64,sc_coarsesyndrome,0},
  {"controller",32,1,1,"CONTROLLER NO.",0,0,0},
  {"unitnumber",33,1,1,"UNIT NO.",0,0,0},
  {"serialid",34,3,1,"UNIT SERIAL NO",0,0,0},
  {"mediaid",35,3,1,"MEDIA ID",0,0,0},
  {"message",36,3,1,"MESSAGE",0,0,0},
  {"m86ehm",37,7,1,"EHMSTS",22,0,rf_m86ehm},
  {"m86ebcs",38,7,1,"EBCS",22,0,rf_m86ebcs},
  {"m86edpsr",39,7,1,"EDPSR",15,0,rf_m86edpsr},
  {"m86ibesr",40,7,1,"IBESR",16,0,rf_m86ibesr},
  {"m86ms1",41,7,1,"MSTAT1",23,0,rf_m86ms1},
  {"m86ms2",42,7,1,"MSTAT2",14,0,rf_m86ms2},
  {"m86mdecc",44,7,1,"MDECC",11,0,rf_m86mdecc},
  {"mcbcnt",45,2,2,"BYTE COUNT",0,0,0},
  {"m86evmqsav",46,2,3,"EVMQSAV",0,0,0},
  {"m86cslint",47,7,1,"CSLINT",15,0,rf_m86cslint},
  {"m86ew1",48,2,3,"EBXWD1",0,0,0},
  {"m86ew2",49,2,3,"EBXWD2",0,0,0},
  {"m86ivasav",50,2,3,"IVASAV",0,0,0},
  {"m86vibasav",51,2,3,"VIBASAV",0,0,0},
  {"m86esasav",52,2,3,"ESASAV",0,0,0},
  {"m86isasav",53,2,3,"ISASAV",0,0,0},
  {"m86cpc",54,2,3,"CPC",0,0,0},
  {"m86merg",55,7,1,"MERG",13,0,rf_m86merg},
  {"m86cshctl",56,7,1,"CSHCTL",3,0,rf_m86cshctl},
  {"m86mear",57,2,3,"MEAR",0,0,0},
  {"m86medr",58,2,3,"MEDR",0,0,0},
  {"m86fber",59,7,1,"FBXERR",14,0,rf_m86fber},
  {"m86cses",60,7,1,"CSES",6,0,rf_m86cses},
  {"pc",61,2,3,"PC",0,0,0},
  {"psl",62,7,1,"PSL",15,0,rf_psl},
  {"mcksumm",63,7,1,"SUMMARY CODE",1,0,rf_mcksumm},
  {"mc8cpues",64,7,1,"CPU ERR STATUS",13,0,rf_mc8cpues},
  {"mc8upc",65,2,3,"TRAPPED UPC",0,0,0},
  {"mc8vaviba",66,2,3,"VA",0,0,0},
  {"mc8dreg",67,2,3,"DREG",0,0,0},
  {"mc8tb0",68,7,1,"TBER0",16,0,rf_mc8tb0},
  {"mc8tb1",69,7,1,"TBER1",11,0,rf_mc8tb1},
  {"mc8timo",70,2,3,"TIMEOUT REG",0,0,0},
  {"mc8par",71,2,3,"PARITY",0,0,0},
  {"mc8sbier",72,7,1,"SBIERR",13,0,rf_mc8sbier},
  {"mc5va",73,2,3,"VA LAST REF",0,0,0},
  {"mc5erpc",74,2,3,"PC AT ERROR",0,0,0},
  {"mc5mdr",75,7,3,"MDR",0,0,0},
  {"mc5sav",76,7,1,"SMR",4,0,rf_mc5sav},
  {"mc5rdt",77,2,3,"RLTO",0,0,0},
  {"mc5tbg",78,7,1,"TBGPR",5,0,rf_mc5tbg},
  {"mc5cache",79,7,1,"CAER",5,0,rf_mc5cache},
  {"mc5bus",80,7,1,"BER",5,0,rf_mc5bus},
  {"mc5mce",81,7,1,"MCESR",5,0,rf_mc5mce},
  {"mc1pm1",82,7,3,"PARM1",0,0,0},
  {"mc1pm2",83,7,3,"PARM2",0,0,0},
  {"mc1vap",84,2,3,"VAP",0,0,0},
  {"mc1int",85,7,1,"INTERNAL STATE REG",0,0,0},
  {"m88mcs",86,7,1,"MCSTS",4,0,rf_m88mcs},
  {"m88ipc",87,2,3,"IPC",0,0,0},
  {"m88vaviba",88,2,3,"VA",0,0,0},
  {"m88iber",89,7,1,"IBER",13,0,rf_m88iber},
  {"m88cber",90,7,1,"CBER",13,0,rf_m88cber},
  {"m88eber",91,2,3,"EBER",0,0,0},
  {"m88nmf",92,7,1,"NMIFSR",10,0,rf_m88nmf},
  {"m88nme",93,2,3,"NMIEAR",0,0,0},
  {"m82pm1",94,2,1,"PARM1",0,0,0},
  {"m82va",95,2,3,"ERR VA",0,0,0},
  {"m82vap",96,2,3,"VAP",0,0,0},
  {"m82mar",97,2,3,"MEAR",0,0,0},
  {"m82stat",98,7,1,"MCSR",16,0,rf_m82stat},
  {"m82pcf",99,2,3,"FAILURE PC",0,0,0},
  {"m82upc",100,2,3,"FAILURE UPC",0,0,0},
  {"awer",101,7,1,"SBIER",13,0,rf_awer},
  {"awtoa",102,2,3,"TIMO",0,0,0},
  {"awfs",103,7,1,"FSR",12,0,rf_awfs},
  {"awsc",104,7,1,"COMP",8,0,rf_awsc},
  {"awmt",105,7,1,"MR",15,0,rf_awmt},
  {"ioaba",106,2,3,"IOABA",0,0,0},
  {"dmacid",107,2,3,"DMACID",0,0,0},
  {"dmacca",108,2,3,"DMACCA",0,0,0},
  {"dmabid",109,2,3,"DMABID",0,0,0},
  {"dmabca",110,2,3,"DMABCA",0,0,0},
  {"dmaaid",111,2,3,"DMAAID",0,0,0},
  {"dmaaca",112,2,3,"DMAACA",0,0,0},
  {"dmaiid",113,2,3,"DMAIID",0,0,0},
  {"dmaica",114,2,3,"DMAICA",0,0,0},
  {"ioadc",115,7,1,"IOADC",15,0,rf_ioadc},
  {"ioaes",116,7,1,"IOAES",27,0,rf_ioaes},
  {"ioacs",117,7,1,"CSR",6,0,rf_ioacs},
  {"ioacf",118,7,1,"CR",5,0,rf_ioacf},
  {"sbiaer",119,7,1,"SBIERR",5,0,rf_sbiaer},
  {"sbiato",120,2,3,"TOADR",0,0,0},
  {"sbiafs",121,7,1,"SBISTS",15,0,rf_sbiafs},
  {"sbiasc",122,7,1,"SILOCOMP",8,0,rf_sbiasc},
  {"sbiamr",123,7,1,"MAINT",15,0,rf_sbiamr},
  {"ubacf",124,7,1,"UBA CONFIG",15,0,rf_ubacf},
  {"ubacr",125,7,1,"UBA CTRL",10,0,rf_ubacr},
  {"ubasr",126,7,1,"UBA STATUS",17,0,rf_ubasr},
  {"ubadcr",127,7,1,"UBA DCR",14,0,rf_ubadcr},
  {"ubafme",128,2,3,"FMER",0,0,0},
  {"ubafub",129,2,3,"FUBAR",0,0,0},
  {"stripl",130,1,3,"STRAY IPL",0,0,0},
  {"strvec",131,1,3,"STRAY VECTOR",0,0,0},
  {"fltva",132,2,3,"FLTVA",0,0,0},
  {"pncmes",133,3,1,"PANIC MESSAGE",0,0,0},
  {"pncsp",134,2,3,"SP",0,0,0},
  {"structs",135,1,2,"MEM ERR STRUCTS",0,0,0},
  {"ctrlr",136,1,2,"CNTRLR NO",0,0,0},
  {"errtyp",137,4,1,"MEMORY ERROR TYPE",4,sc_errtyp,0},
  {"errcnt",138,1,2,"NO. ERRS ON THIS ADDR",0,0,0},
  {"memer0",139,7,1,"CSRA",21,0,rf_memer0},
  {"memer1",140,7,1,"CSRB",10,0,rf_memer1},
  {"memer2",141,7,1,"CSRC",10,0,rf_memer2},
  {"memer3",142,7,1,"CSRD",10,0,rf_memer3},
  {"pncap",143,2,3,"AP",0,0,0},
  {"pncfp",144,2,3,"FP",0,0,0},
  {"pncksp",145,2,3,"KSP",0,0,0},
  {"pncusp",146,2,3,"USP",0,0,0},
  {"pncisp",147,2,3,"ISP",0,0,0},
  {"pncp0b",148,2,3,"P0BR",0,0,0},
  {"pncp0l",149,2,3,"P0LR",0,0,0},
  {"p1br",150,2,3,"P1BR",0,0,0},
  {"p1lr",151,2,3,"P1LR",0,0,0},
  {"pncp1b",152,2,3,"P1BR",0,0,0},
  {"pncp1r",153,2,3,"P1LR",0,0,0},
  {"pncsbr",154,2,3,"SBR",0,0,0},
  {"pncslr",155,2,3,"SLR",0,0,0},
  {"pncpcb",156,2,3,"PCBB",0,0,0},
  {"pncscb",157,2,3,"SCBB",0,0,0},
  {"pncipl",158,2,3,"IPL",0,0,0},
  {"pncast",159,2,3,"ASTR",0,0,0},
  {"pncsis",160,2,3,"SISR",0,0,0},
  {"pncicc",161,2,3,"ICCS",0,0,0},
  {"cpunum",162,1,2,"CPU NUMBER",0,0,0},
  {"bibusnum",163,2,2,"BI BUS NUMBER",0,0,0},
  {"bidevreg",164,7,1,"BI DEVICE REG",2,0,rf_bidevreg},
  {"bicsr",165,7,1,"BI CSR",14,0,rf_bicsr},
  {"bibuserreg",166,7,1,"BI 0 BUS ERR REG",21,0,rf_bibuserreg},
  {"bierint",167,7,1,"BI ERROR ICR",10,0,rf_bierint},
  {"biintdst",168,7,1,"INTRPT DEST REG",2,0,rf_biintdst},
  {"bibsnum",169,1,2,"BI BUS NUMBER",0,0,0},
  {"buacsr",170,7,1,"BUA CSR",13,0,rf_buacsr},
  {"buafub",171,2,3,"BUA FUBAR",0,0,0},
  {"majmin",172,1,2,"DEVICE MAJOR/MINOR NUMBERS",0,0,0},
  {"trsbcnt",173,2,2,"TRANSFER BYTE COUNT",0,0,0},
  {"blkno",174,2,2,"LBN",0,0,0},
  {"retrycnt",175,1,2,"RETRY COUNT",0,0,0},
  {"hard_errs",176,1,2,"HARD ERROR COUNT",0,0,0},
  {"soft_errs",177,1,2,"SOFT ERROR COUNT",0,0,0},
  {"devcsr",178,1,3,"CSR",0,0,0},
  {"cmdref",179,2,3,"COMMAND REF NUMBER",0,0,0},
  {"dsaunit",180,1,2,"UNIT NUMBER",0,0,0},
  {"dsaseq",181,1,2,"PACKET SEQUENCE NUMBER",0,0,0},
  {"dsaformat",182,6,1,"FORMAT",1,0,rf_dsaformat},
  {"dsaflags",183,6,1,"FLAGS",7,0,rf_dsaflags},
  {"event_code",184,6,1,"EVENT CODE",2,0,rf_event_code},
  {"dsastat",185,6,1,"STATUS AND EVENT CODE",1,0,rf_dsastat},
  {"cntrlidnum",186,9,3,"UNIQUE CONTR ID",6,0,0},
  {"cntrlid",187,6,1,"CONTROLLER TYPE",2,0,rf_cntrlid},
  {"csvrsn",188,1,2,"CONTROLLER SOFTWARE REV",0,0,0},
  {"chvrsn",189,1,2,"CONTROLLER HARDWARE REV",0,0,0},
  {"multi_unit",190,1,3,"MULTI-UNIT CODE",0,0,0},
  {"unitid1",191,9,3,"UNIQUE UNIT NUMBER",6,0,0},
  {"hmemaddr",192,2,3,"HOST MEMORY ADDRESS",0,0,0},
  {"unitid2",193,6,1,"UNIT ID",2,0,rf_unitid2},
  {"usvrsn",194,1,2,"UNIT SOFTWARE REV",0,0,0},
  {"ushvrsn",195,1,2,"UNIT HARDWARE REV",0,0,0},
  {"cylinder",196,1,2,"CYLINDER",0,0,0},
  {"std_retlvl",197,6,1,"RETRY/LEVEL INFO",2,0,rf_std_retlvl},
  {"replflg",198,6,1,"BAD BLK RPLCMNT FLAG",7,0,rf_replflg},
  {"volsernum",199,2,3,"VOLUME SERIAL NUMBER",0,0,0},
  {"hdrcod",200,2,3,"HEADER CODE",0,0,0},
  {"sd1stat",201,7,1,"SDI STATUS WORD 1",21,0,rf_sd1stat},
  {"badlbn",202,2,2,"BAD LBN",0,0,0},
  {"sd2stat",203,7,1,"SDI STATUS WORD 2",3,0,rf_sd2stat},
  {"oldrbn",204,2,2,"PREVIOUS RBN",0,0,0},
  {"sd3stat",205,7,1,"SDI STATUS WORD 3",3,0,rf_sd3stat},
  {"newrbn",206,2,2,"NEW RBN",0,0,0},
  {"cause",207,1,3,"BAD BLOCK REPL CAUSE",0,0,0},
  {"sdireqbyte",208,6,1,"REQUEST BYTE",8,0,rf_sdireqbyte},
  {"sdimodbyte",209,6,1,"MODE BYTE",6,0,rf_sdimodbyte},
  {"sdierrbyte",210,6,1,"ERROR BYTE",6,0,rf_sdierrbyte},
  {"sdictlbyte",211,6,1,"CONTROLLER BYTE",1,0,rf_sdictlbyte},
  {"sdiret",212,1,2,"RETRY CNT/FAILURE CODE",0,0,0},
  {"ra6prvcyl",213,9,2,"PREVIOUS CYLINDER",2,0,0},
  {"ra6prvhd",214,1,2,"PREVIOUS HEAD",0,0,0},
  {"curcyl",215,9,2,"CURRENT CYLINDER",2,0,0},
  {"curhd",216,1,2,"CURRENT HEAD",0,0,0},
  {"sdilstop",217,6,1,"LAST OPCODE",1,0,rf_sdilstop},
  {"sdidrvdet",218,6,1,"SDI ERR INFO",6,0,rf_sdidrvdet},
  {"led",219,1,3,"LED CODE",0,0,0},
  {"fpcod",220,1,3,"FRONT PANEL CODE",0,0,0},
  {"tp_event_code",221,6,1,"EVENT CODE",2,0,rf_tp_event_code},
  {"tp_dsastat",222,6,1,"STATUS AND EVENT CODE",1,0,rf_tp_dsastat},
  {"tp_unitid2",223,7,1,"UNIT ID",3,0,rf_tp_unitid2},
  {"position",224,2,3,"POSITION AT ERROR",0,0,0},
  {"fmt_svr",225,1,2,"FORMAT SOFTWARE REV",0,0,0},
  {"fmt_hvr",226,1,2,"FORMAT HARDWARE REV",0,0,0},
  {"st1drvst",227,7,1,"DRIVE CHARACTERISTICS",3,0,rf_st1drvst},
  {"gap_count",228,2,2,"GAP COUNT",0,0,0},
  {"st3drvst",229,2,3,"DEVICE DEPENDENT DATA",0,0,0},
  {"st4drvst",230,2,3," ",0,0,0},
  {"st5drvst",231,2,3," ",0,0,0},
  {"st6drvst",232,2,3," ",0,0,0},
  {"st7drvst",233,2,3," ",0,0,0},
  {"st8drvst",234,2,3," ",0,0,0},
  {"st9drvst",235,2,3," ",0,0,0},
  {"st10drvst",236,2,3," ",0,0,0},
  {"st11drvst",237,2,3," ",0,0,0},
  {"st12drvst",238,2,3," ",0,0,0},
  {"st13drvst",239,2,3," ",0,0,0},
  {"st14drvst",240,2,3," ",0,0,0},
  {"st15drvst",241,2,3," ",0,0,0},
  {"st16drvst",242,2,3," ",0,0,0},
  {"stisumm",243,7,1,"STI SUMMARY",19,0,rf_stisumm},
  {"stidrv0",244,6,1,"DRIVE 0 INFORMATION",14,0,rf_stidrv0},
  {"stidrv1",245,6,1,"DRIVE 1 INFO",14,0,rf_stidrv1},
  {"stidrv2",246,6,1,"DRIVE 2 INFO",14,0,rf_stidrv2},
  {"stidrv3",247,6,1,"DRIVE 3 INFO",14,0,rf_stidrv3},
  {"stifmterr1",248,2,3,"GET XTNDED FMT STATUS",0,0,0},
  {"stifmterr2",249,2,3," ",0,0,0},
  {"stifmterr3",250,2,3," ",0,0,0},
  {"stifmterr4",251,2,3," ",0,0,0},
  {"stifmterr5",252,2,3," ",0,0,0},
  {"dsavar1",253,2,3,"CONTROLLER DEPENDENT INFO",0,0,0},
  {"dsavar2",254,2,3," ",0,0,0},
  {"dsavar3",255,2,3," ",0,0,0},
  {"dsavar4",256,2,3," ",0,0,0},
  {"dsavar5",257,2,3," ",0,0,0},
  {"dsavar6",258,2,3," ",0,0,0},
  {"dsavar7",259,2,3," ",0,0,0},
  {"dsavar8",260,2,3," ",0,0,0},
  {"dsavar9",261,2,3," ",0,0,0},
  {"dsavar10",262,2,3," ",0,0,0},
  {"dsavar11",263,2,3," ",0,0,0},
  {"dsavar12",264,2,3," ",0,0,0},
  {"dsavar13",265,2,3," ",0,0,0},
  {"dsavar14",266,2,3," ",0,0,0},
  {"dsavar15",267,2,3," ",0,0,0},
  {"dsavar16",268,2,3," ",0,0,0},
  {"dsavar17",269,2,3," ",0,0,0},
  {"dsavar18",270,2,3," ",0,0,0},
  {"dsavar19",271,2,3," ",0,0,0},
  {"dsavar20",272,2,3," ",0,0,0},
  {"orig_err",273,1,3,"ORIGINAL ERR FLGS",0,0,0},
  {"err_rec_flags",274,1,3,"ERROR RECVRY FLGS",0,0,0},
  {"lvl_a_retry",275,1,2,"LEVEL A RETRY",0,0,0},
  {"lvl_b_retry",276,1,2,"LEVEL B RETR",0,0,0},
  {"buf_data_mem_addr",277,1,3,"BUFF DATA MEM ADDR",0,0,0},
  {"cont_addl_info",278,1,3,"ADDITIONAL INFO",0,0,0},
  {"m82summ",279,7,1,"SUMMARY CODE",9,0,rf_m82summ},
  {"m8cmera",280,7,1,"CSRA",15,0,rf_m8cmera},
  {"mc8merb",281,7,1,"CSRB",9,0,rf_mc8merb},
  {"m8cmerc",282,7,1,"CSRC",6,0,rf_m8cmerc},
  {"m5mer0",283,7,1,"CSR0",7,0,rf_m5mer0},
  {"m5mer1",284,7,1,"CSR1",9,0,rf_m5mer1},
  {"m5mer2",285,7,1,"CSR2",5,0,rf_m5mer2},
  {"m88csr0",286,7,1,"CSR0",10,0,rf_m88csr0},
  {"m88csr1",287,7,1,"CSR1",11,0,rf_m88csr1},
  {"m88csr2",288,7,1,"CSR2",16,0,rf_m88csr2},
  {"m88csr3",289,7,1,"CSR3",16,0,rf_m88csr3},
  {"m82csr0",290,7,1,"CSR0",14,0,rf_m82csr0},
  {"m82csr1",291,7,1,"CSR1",8,0,rf_m82csr1},
  {"blacsr",292,7,1,"BLA CSR",13,0,rf_blacsr},
  {"cierrcod",293,7,1,"REASON FOR ERROR",1,0,rf_cierrcod},
  {"cipcnf",294,7,1,"PORT CONFIG REG",18,0,rf_cipcnf},
  {"cipmcsr",295,7,1,"PORT MAINT CSR",17,0,rf_cipmcsr},
  {"cipsr",296,7,1,"PORT STATUS REG",9,0,rf_cipsr},
  {"cippr",297,7,1,"PORT PARAM REG",5,0,rf_cippr},
  {"bicsr1",298,7,1,"BI1 CSR",14,0,rf_bicsr1},
  {"nbiacsr0",299,7,1,"NMI BIA #0 CSR0",18,0,rf_nbiacsr0},
  {"nbia1csr0",300,7,1,"NMI BIA #1 CSR0",18,0,rf_nbia1csr0},
  {"nbiacsr1",301,7,1,"NMI BIA #0 CSR1",14,0,rf_nbiacsr1},
  {"bi1buserreg",302,7,1,"BI 1 BUS ERROR REG",21,0,rf_bi1buserreg},
  {"ciaddr",303,2,3,"BAD UCODE ADDRESS",0,0,0},
  {"cibvalue",304,2,3,"BAD UCODE VALUE",0,0,0},
  {"cigvalue",305,2,3,"GOOD UCODE VALUE",0,0,0},
  {"cilsaddr",306,9,3,"LOCAL STATION ADDR",6,0,0},
  {"cilsysid",307,9,2,"LOCAL STATION ID",6,0,0},
  {"cilname",308,3,1,"LOCAL NODE",0,0,0},
  {"cirsaddr",309,9,3,"REMOTE STATION ADDR",6,0,0},
  {"cirsysid",310,9,2,"REMOTE STATION ID",6,0,0},
  {"cirname",311,3,1,"REMOTE NODE",0,0,0},
  {"ciport",312,1,2,"DESTINATION PORT",0,0,0},
  {"cistatus",313,1,3,"STATUS",0,0,0},
  {"ciopcode",314,1,3,"OPCODE",0,0,0},
  {"ciflags",315,1,3,"PORT CMD FLAGS",0,0,0},
  {"cilocal",316,1,2,"LOCAL CI PPD VRSN",0,0,0},
  {"ciremote",317,1,2,"REMOTE CI PPD VRSN",0,0,0},
  {"ciksysid",318,9,2,"KNOWN SYS ID",6,0,0},
  {"cikname",319,3,1,"KNOWN NODE",0,0,0},
  {"biaddr",320,2,3,"BI TRANSACT ADDR",0,0,0},
  {"bvpdata",321,2,3,"PORT DATA",0,0,0},
  {"ciper",322,7,1,"PORT ERROR REGISTER",1,0,rf_ciper},
  {"bvpsts",323,7,1,"PORT STATUS REG",13,0,rf_bvpsts},
  {"bvperr",324,7,1,"PORT ERROR REG",2,0,rf_bvperr},
  {"cibcipcnf",325,7,1,"BICA CONFIG REG",12,0,rf_cibcipcnf},
  {"cibcapmcsr",326,7,1,"PORT MAINT CSR",16,0,rf_cibcapmcsr},
  {"cibcapsr",327,7,1,"PORT STATUS REG",14,0,rf_cibcapsr},
  {"busnum",329,1,2,"BUS NUMBER",0,0,0},
  {"sareg",330,7,1,"SA REGISTER",0,0,0},
  {"cierrs",331,1,2,"NUMBER OF ERRORS",0,0,0},
  {"cireinits",332,1,2,"NUMBER OF REINITS",0,0,0},
  {"cipfaddr",333,2,3,"PORT FAILING ADDR",0,0,0},
  {"cibcappr",334,7,1,"PORT PARAMETER REG",8,0,rf_cibcappr},
  {"bvpcntl",335,7,1,"PORT CONTROL REG",3,0,rf_bvpcntl},
  {"pnckrnstk",337,12,7,"KERNEL STACK",128,0,0},
  {"pncintstk",338,12,7,"INTERRUPT STACK",128,0,0},
  {"nmifltsilo",339,11,7,"SILO REGS",256,0,0},
  {"sbiawsilo",340,11,7,"SILO REGS",16,0,0},
  {"sbiawcsr",341,11,7,"CSR REGS",16,0,0},
  {"mvaxmser",342,7,1,"MSER",10,0,rf_mvaxmser},
  {"mvaxcaer",343,7,1,"CEAR",0,0,0},
  {"mvaxdaer",344,7,1,"DEAR",0,0,0},
  {"sxerr_csr",345,7,1,"ACP STATUS REG",10,0,rf_sxerr_csr},
  {"vxstrmser",346,7,1,"MSER",7,0,rf_vxstrmser},
  {"uda5sa",347,7,1,"STAT ADDR REG",2,0,rf_uda5sa},
  {"kdb5sa",348,7,1,"STATUS ADDR REG",2,0,rf_kdb5sa},
  {"rqdx3sa",349,7,1,"STAT ADDR REG",2,0,rf_rqdx3sa},
  {"mcuv1summ",350,7,1,"SUMMARY CODE",1,0,rf_mcuv1summ},
  {"mcuv2summ",351,7,1,"SUMMARY CODE",1,0,rf_mcuv2summ},
  {"mc73summ",352,7,1,"SUMMARY CODE",1,0,rf_mc73summ},
  {"sdirtpi",353,6,1,"REAL TIME PRT IMAGE",9,0,rf_sdirtpi},
  {"sdidrvst",354,6,1,"DRIVE STATE",8,0,rf_sdidrvst},
  {"sxerr_diagreg",355,7,1,"DIAGNOSTIC REG",0,0,0},
  {"sxerrsubtst",356,2,2,"FAILING SUBTEST #",0,0,0},
  {"sxerrinfo",357,11,3,"ERROR INFORMATION",10,0,0},
  {"tk57sa",358,7,1,"STATUS ADDR REG",2,0,rf_tk57sa},
  {"rc25sa",359,7,1,"STATUS ADDR REG",1,0,rf_rc25sa},
  {"ctlrvar1",360,1,3,"CONTROLLER DEPENDENT INFO",0,0,0},
  {"ciprotaddl",361,11,7,"ADDL INFO",20,0,0},
  {"ciaddlprot",362,1,3," ",0,0,0},
  {"cicolladdl",363,11,7,"ADDL INFO",20,0,0},
  {"cilpktlpaddl",364,11,7,"ADDL INFO",20,0,0} };

/*************** STD_SEG_ELE_STRUCT ******************/

  short   seg_2 [] = { 27,21,22,23,24,25,26,28};
  short   seg_3 [] = { 29,30,32,33,34,35,31};
  short   seg_4 [] = { 36};
  short   seg_5 [] = { 37,38,39,40,41,42,44,60};
  short   seg_6 [] = { 63,65,66,69,70,71,72};
  short   seg_7 [] = { 63,73,78,79,80};
  short   seg_8 [] = { 350};
  short   seg_9 [] = { 351};
  short   seg_10 [] = { 89,90,91,92};
  short   seg_11 [] = { 279,98};
  short   seg_12 [] = { 103};
  short   seg_13 [] = { 103};
  short   seg_14 [] = { 116,118,119,120,121,340};
  short   seg_15 [] = { 124,126,128,129};
  short   seg_16 [] = { 133};
  short   seg_17 [] = { 139,140,141,142};
  short   seg_18 [] = { 45,46,47,48,49,50,51,52,53,
			54,55,56,57,58,59,61,62};
  short   seg_19 [] = { 45,64,67,68,61,62};
  short   seg_20 [] = { 45,74,75,76,77,81,61,62};
  short   seg_21 [] = { 45,82,83,61,62};
  short   seg_22 [] = { 45,84,85,61,62};
  short   seg_23 [] = { 45,86,87,88,93,61,62};
  short   seg_24 [] = { 45,94,95,96,97,99,100,61,62};
  short   seg_25 [] = { 101,102,104,105,340,341,61,62};
  short   seg_26 [] = { 101,102,104,105,340,341,61,62};
  short   seg_27 [] = { 106,107,108,109,110,111,112,113,114,
			115,117,122,123,341,61,62};
  short   seg_28 [] = { 125,127,61,62};
  short   seg_29 [] = { 130,131};
  short   seg_30 [] = { 132,61,62};
  short   seg_31 [] = { 136,138};
  short   seg_32 [] = { 134,143,144,61,145,146,147,148,149,
			152,151,154,155,156,157,158,159,160,161};
  short   seg_33 [] = { 164,165,166,170};
  short   seg_34 [] = { 170};
  short   seg_35 [] = { 171,61,62};
  short   seg_36 [] = { 61,62};
  short   seg_37 [] = { 164,165,166,167,168};
  short   seg_38 [] = { 172,173,174,175,176,177,180};
  short   seg_39 [] = { 192};
  short   seg_40 [] = { 172,173,174,175,176,177,192};
  short   seg_41 [] = { 186,187,184,185};
  short   seg_42 [] = { 172,173,174,175,176,177};
  short   seg_43 [] = { 172,173,174,175,176,177,213,215,214,
			216,220};
  short   seg_44 [] = { 172,173,174,175,176,177,218,215,216,
			219,220};
  short   seg_45 [] = { 172,173,174,175,176,177,196};
  short   seg_46 [] = { 172,173,174,175,176,177,202,204,206,
			207};
  short   seg_47 [] = { 172,173,174,175,176,177,224};
  short   seg_48 [] = { 172,173,174,175,176,177,224,243};
  short   seg_49 [] = { 172,173,174,175,176,177,224};
  short   seg_50 [] = { 172,173,174,175,176,177,224};
  short   seg_51 [] = { 178,182,183,184,185};
  short   seg_52 [] = { 178,182,183,221,222};
  short   seg_53 [] = { 182,183};
  short   seg_54 [] = { 178,182,183,221,222,224};
  short   seg_55 [] = { 178,182,183,184,185,200};
  short   seg_56 [] = { 178,182,183,184,185,217};
  short   seg_57 [] = { 178,182,183,184,185};
  short   seg_58 [] = { 178,182,183,184,185,200};
  short   seg_59 [] = { 178,182,183,184,185,198};
  short   seg_60 [] = { 178,182,183,221,222};
  short   seg_61 [] = { 178,182,183,221,222,244,245,246,247};
  short   seg_62 [] = { 178,182,183,221,222};
  short   seg_63 [] = { 178,182,183,221,222};
  short   seg_64 [] = { 179,180,181,186,187,188,189,190,191,
			194,195};
  short   seg_65 [] = { 193};
  short   seg_66 [] = { 223};
  short   seg_67 [] = { 179,181,186,187,188,189};
  short   seg_68 [] = { 253,254,255,256,257,258,259,260,261,
			262,263,264,265,266,267,268,269,270,271,
			272};
  short   seg_69 [] = { 179,181,188,189};
  short   seg_70 [] = { 197,273,274,275,276,277,278};
  short   seg_71 [] = { 208,209,210,211,199,212};
  short   seg_72 [] = { 199,253,254,255,256,257,258,259,260,
			261,262,263,264,265,266,267,268,269,270,
			271,272};
  short   seg_73 [] = { 197,199,273,274,275,276,277,278};
  short   seg_74 [] = { 199};
  short   seg_75 [] = { 197,225,226,229,254,255,256,257,258,
			259,260,261,262,263,264,265,266,267,268,
			269,270,271,272};
  short   seg_76 [] = { 225,226};
  short   seg_77 [] = { 225,226,227,228,229,230,231,232,233,
			234,235,236,237,238,239,240,241,242};
  short   seg_78 [] = { 225,226,248,249,250,251,252};
  short   seg_79 [] = { 280,281,282};
  short   seg_80 [] = { 139,140,141,142};
  short   seg_81 [] = { 283,284,285};
  short   seg_82 [] = { 44,57,41,42};
  short   seg_83 [] = { 286,287,288,289};
  short   seg_84 [] = { 290,291};
  short   seg_85 [] = { 337};
  short   seg_86 [] = { 338};
  short   seg_87 [] = { 92,286,93};
  short   seg_88 [] = { 299,300};
  short   seg_89 [] = { 339};
  short   seg_90 [] = { 299,301};
  short   seg_91 [] = { 166,302};
  short   seg_92 [] = { 323,166,320};
  short   seg_93 [] = { 323,324,321};
  short   seg_94 [] = { 164,165,335};
  short   seg_95 [] = { 36};
  short   seg_96 [] = { 169,164,292};
  short   seg_97 [] = { 165,166};
  short   seg_98 [] = { 61,62};
  short   seg_99 [] = { 293};
  short   seg_100 [] = { 293};
  short   seg_101 [] = { 293};
  short   seg_102 [] = { 331,332,294,296,297,295,333};
  short   seg_103 [] = { 331,332,325,296,322,295,333,297};
  short   seg_104 [] = { 331,332,327,322,326,327,334};
  short   seg_105 [] = { 303,304,305};
  short   seg_106 [] = { 164,165,166,167,168,303,304,305};
  short   seg_107 [] = { 293};
  short   seg_108 [] = { 312,313,314,315,364};
  short   seg_109 [] = { 316,317,361,362};
  short   seg_110 [] = { 318,319,363};
  short   seg_111 [] = { 293,331,332};
  short   seg_112 [] = { 164,165,166,167,168};
  short   seg_113 [] = { 331,332,306,307,308,309,310,311};
  short   seg_114 [] = { 342,343,344};
  short   seg_115 [] = { 330,329};
  short   seg_116 [] = { 347};
  short   seg_117 [] = { 348};
  short   seg_118 [] = { 352};
  short   seg_119 [] = { 45,82,83,61,62};
  short   seg_120 [] = { 172,173,174,175,176,177,353,215,216,
			219,220};
  short   seg_121 [] = { 172,173,174,175,176,177,354,215,216,
			219,220};
  short   seg_122 [] = { 172,173,174,175,176,177,215,216,219,
			220};
  short   seg_123 [] = { 345};
  short   seg_124 [] = { 355};
  short   seg_125 [] = { 356,357};
  short   seg_126 [] = { 359};
  short   seg_127 [] = { 358};
  short   seg_128 [] = { 360,254,255,256};
  short   seg_129 [] = { 253,254,255,256};
  short   seg_130 [] = { 257,258,259,260};
  short   seg_131 [] = { 261,262,263,264};
  short   seg_132 [] = { 265,266,267,268};
  short   seg_133 [] = { 269,270,271,272};
DD$STD_SEGMENT_DSD std_seg_dsd_struct[] = { 
  {1,1,"EVENT INFORMATION SEGMENT",8,seg_2,51},
  {2,1,"UNIT INFORMATION",7,seg_3,46},
  {4,1," ",1,seg_4,16},
  {4,4,"MCK INFO",8,seg_5,51},
  {4,9,"MCK INFO",7,seg_6,46},
  {4,2,"MCK INFO",5,seg_7,36},
  {4,7,"MCK INFO",1,seg_8,16},
  {4,8,"MCK INFO",1,seg_9,16},
  {4,6,"MCK INFO",4,seg_10,31},
  {4,5,"MCK INFO",2,seg_11,21},
  {4,10,"780 SBI INFO",1,seg_12,16},
  {4,11,"ASYNCHRONOUS WRITE INFO",1,seg_13,16},
  {4,12,"8600 SBIA INFO",6,seg_14,41},
  {4,13,"780 UBA INFO",4,seg_15,31},
  {4,14,"PANIC ",1,seg_16,16},
  {4,15,"MEMORY ERROR INFO",4,seg_17,31},
  {5,1,"ADDITIONAL MCK INFORMATION",17,seg_18,96},
  {5,2,"ADDITIONAL MCK INFORMATION",6,seg_19,41},
  {5,3,"ADDITIONAL MCK INFO",8,seg_20,51},
  {5,4,"ADDITIONAL MCK INFO",5,seg_21,36},
  {5,5,"ADDITIONAL MCK INFO",5,seg_22,36},
  {5,6,"ADDITIONAL MCK INFO",7,seg_23,46},
  {5,7,"ADDITIONAL MCK INFO",9,seg_24,56},
  {5,8,"ADDITIONAL SBI INFO",8,seg_25,51},
  {5,9,"ADDITIONAL INFORMATION",8,seg_26,51},
  {5,10,"ADDITIONAL INFO",16,seg_27,91},
  {5,11,"ADDITIONAL INFO",4,seg_28,31},
  {5,12,"ADDITIONAL INFO",2,seg_29,21},
  {5,13,"ADDITIONAL INFO",3,seg_30,26},
  {5,15,"ADDITIONAL MEMORY INFO",2,seg_31,21},
  {5,14,"SYSTEM REGISTERS",19,seg_32,106},
  {4,16,"BI/BUA INFORMATION",4,seg_33,31},
  {3,1,"BUA DEVICE INFORMATION",1,seg_34,16},
  {5,16,"ADDITIONAL BUA INFO",3,seg_35,26},
  {4,17,"BI BUS INFO",2,seg_36,21},
  {5,17,"BI ERR REGISTERS",5,seg_37,36},
  {4,18,"DISK TRANSFER INFO",7,seg_38,46},
  {4,19,"DSA MEMORY ERROR INFO",1,seg_39,16},
  {4,20,"DSA TAPE MEMORY ERROR INFO",7,seg_40,46},
  {4,21,"DSA DISK CONTROLLER ERR INFO",4,seg_41,31},
  {4,22,"DSA TAPE CONTROLLER ERR INFO",6,seg_42,41},
  {4,23,"SDI ERROR INFO",11,seg_43,66},
  {4,24,"SDI ERROR INFO",11,seg_44,66},
  {4,25,"SMALL DISK ERR INFO",7,seg_45,46},
  {4,26,"BAD BLOCK RPLCMNT INFO",10,seg_46,61},
  {4,27,"TAPE TRANSFER ERR INFO",7,seg_47,46},
  {4,28,"STI COMMUNIC/CMD FAIL INFO",8,seg_48,51},
  {4,29,"STI DRIVE ERROR INFO",7,seg_49,46},
  {4,30,"STI GET FMTR ERR INFO",7,seg_50,46},
  {3,2,"SUPPORTING DATA",5,seg_51,36},
  {3,3,"SUPPORTING DATA",5,seg_52,36},
  {3,4,"SUPPORTING DATA",2,seg_53,21},
  {3,5,"SUPPORTING DATA",6,seg_54,41},
  {3,6,"SUPPORTING DATA",6,seg_55,41},
  {3,7,"SUPPORTING DATA",6,seg_56,41},
  {3,8,"SUPPORTING DATA",5,seg_57,36},
  {3,9,"SUPPORTING DATA",6,seg_58,41},
  {3,10,"SUPPORTING DATA",6,seg_59,41},
  {3,11,"SUPPORTING DATA",5,seg_60,36},
  {3,12,"SUPPORTING DATA",9,seg_61,56},
  {3,13,"SUPPORTING DATA",5,seg_62,36},
  {3,14,"SUPPORTING DATA",5,seg_63,36},
  {5,18,"ADDITIONAL COMMON INFO",11,seg_64,66},
  {5,19," ",1,seg_65,16},
  {5,20," ",1,seg_66,16},
  {5,21," ",6,seg_67,41},
  {5,22," ",20,seg_68,111},
  {5,23,"ADDITIONAL INFO",4,seg_69,31},
  {5,24," ",7,seg_70,46},
  {5,25," ",6,seg_71,41},
  {5,26," ",21,seg_72,116},
  {5,27," ",8,seg_73,51},
  {5,28," ",1,seg_74,16},
  {5,29," ",23,seg_75,126},
  {5,30," ",2,seg_76,21},
  {5,31," ",18,seg_77,101},
  {5,32," ",7,seg_78,46},
  {4,31,"780-C CSR REGS",3,seg_79,26},
  {4,32,"780-E CSR REGS",4,seg_80,31},
  {4,33,"750 CSR REGS",3,seg_81,26},
  {4,34,"8600 MEMORY ERROR REGS",4,seg_82,31},
  {4,35,"8800 MEMORY CSR REGS",4,seg_83,31},
  {4,36,"8200 MEMORY CSR REGS",2,seg_84,21},
  {5,33," ",1,seg_85,16},
  {5,37," ",1,seg_86,16},
  {3,14,"NMI FAULT STATUS INFO",3,seg_87,26},
  {4,37,"NMI FAULT BI INFORMATION",2,seg_88,21},
  {5,41,"NMI SILO REGISTERS",1,seg_89,16},
  {4,38,"NMI ADAPTER CSRS",2,seg_90,21},
  {3,16,"NMI ADAPTER BI BUS REGS",2,seg_91,21},
  {4,39,"BVP BI ERROR INFO",3,seg_92,26},
  {4,40,"BVP ERROR REGS",3,seg_93,26},
  {3,17,"OTHER BVP ERROR INFO",3,seg_94,26},
  {4,41,"STARTUP INFO",1,seg_95,16},
  {4,42,"BLA ERROR CDS",3,seg_96,26},
  {3,18,"BI INFO",2,seg_97,21},
  {5,49,"ADDITIONAL INFO",2,seg_98,21},
  {4,43,"ERROR CODE INFO",1,seg_99,16},
  {4,44,"ERROR CODE INFO",1,seg_100,16},
  {4,45,"ERROR CODE INFO",1,seg_101,16},
  {3,19,"SUPPORTING DATA ATTN INFO",7,seg_102,46},
  {3,20,"SUPPORTING DATA ATTN INFO",8,seg_103,51},
  {3,21,"SUPPORTING DATA ATTN INFO",7,seg_104,46},
  {5,50,"ADDITIONAL DATTN INFO",3,seg_105,26},
  {5,51,"ADDITIONAL DATTN INFO",8,seg_106,51},
  {4,46,"ERROR CODE INFO",1,seg_107,16},
  {5,52,"LOGGED PKT PKT INFO",5,seg_108,36},
  {5,54,"LOGGED PACKET PROTOCOL INFO",4,seg_109,31},
  {5,55,"LOGGED PKT COLLISION INFO",3,seg_110,26},
  {4,47,"COMMON CI INFO",3,seg_111,26},
  {3,22,"BI CI COMMON INFO",5,seg_112,36},
  {3,23,"SUPPORTING LOG PKT INFO",8,seg_113,51},
  {4,48,"MEMORY ERR REGS",3,seg_114,26},
  {4,49,"UQSSP ATTN INFO",2,seg_115,21},
  {4,50,"STATUS ADDRESS INFO",1,seg_116,16},
  {4,51,"STATUS ADDRESS INFO",1,seg_117,16},
  {4,52,"SUMMARY INFO",1,seg_118,16},
  {5,56,"ADDITIONAL MCK INFO",5,seg_119,36},
  {4,53,"SDI ERROR INFO",11,seg_120,66},
  {4,54,"SDI ERROR INFO",11,seg_121,66},
  {4,55,"SDI ERROR INFO",10,seg_122,61},
  {4,56,"CSR INFO",1,seg_123,16},
  {3,24,"DIAGNOSTIC INFO",1,seg_124,16},
  {5,57," ",2,seg_125,21},
  {4,57,"STATUS ADDR INFO",1,seg_126,16},
  {4,58,"STATUS ADDR INFO",1,seg_127,16},
  {5,58," ",4,seg_128,31},
  {5,59," ",4,seg_129,31},
  {5,60," ",4,seg_130,31},
  {5,61," ",4,seg_131,31},
  {5,62," ",4,seg_132,31},
  {5,63," ",4,seg_133,31} };

/***************** OS_CODE_STRUCT *********************/

DD$OS_CODE  oi_5[] = { 
  {100,1},
  {101,1},
  {102,1},
  {103,1},
  {104,1},
  {105,1},
  {106,1},
  {107,1},
  {108,1},
  {109,1},
  {110,1},
  {111,1},
  {112,1},
  {200,1},
  {251,1},
  {252,1},
  {300,2},
  {301,2},
  {250,2},
  {310,2},
  {350,3},
  {351,3} };
DD$OS_CODE  oi_6[] = { 
  {100,1},
  {107,1},
  {251,1},
  {252,1} };
DD$OS_CODE  oi_7[] = { 
  {101,2},
  {108,2} };
DD$OS_CODE  oi_8[] = { 
  {1,3},
  {2,3},
  {3,3},
  {4,3},
  {5,3},
  {6,3},
  {7,3},
  {8,3},
  {9,3},
  {10,3},
  {11,3},
  {12,3},
  {13,3},
  {14,3},
  {15,3},
  {16,3},
  {17,3},
  {18,3},
  {19,3} };
DD$OS_CODE  oi_9[] = { 
  {3,4},
  {4,4},
  {5,4},
  {6,4},
  {7,4},
  {8,4},
  {9,4} };
DD$OS_CODE  oi_10[] = { 
  {1,5},
  {2,5},
  {3,5},
  {4,5},
  {14,5},
  {15,5} };
DD$OS_CODE  oi_11[] = { 
  {3,6},
  {4,6},
  {5,6},
  {7,6},
  {8,6},
  {10,6} };
DD$OS_CODE  oi_12[] = { 
  {106,7} };
DD$OS_CODE  oi_13[] = { 
  {104,8},
  {105,8} };
DD$OS_CODE  oi_14[] = { 
  {110,15},
  {111,15} };
DD$OS_CODE  oi_15[] = { 
  {4,2},
  {6,3},
  {5,4},
  {1,6},
  {2,7},
  {7,8},
  {8,9},
  {3,150} };
DD$OS_CODE  oi_16[] = { 
  {1,2},
  {2,3} };
DD$OS_CODE  oi_17[] = { 
  {5,15},
  {3,16},
  {2,17} };
DD$OS_CODE  oi_18[] = { 
  {4,18},
  {1,19},
  {5,20},
  {11,21},
  {12,25},
  {6,26},
  {8,27},
  {9,28},
  {13,29},
  {7,30},
  {10,31},
  {2,62},
  {3,112},
  {14,113},
  {15,114},
  {17,115},
  {18,116},
  {19,117} };
DD$OS_CODE  oi_19[] = { 
  {3,38},
  {4,39},
  {5,40},
  {6,41},
  {9,48},
  {8,49} };
DD$OS_CODE  oi_20[] = { 
  {1,32},
  {4,33},
  {2,35},
  {3,34} };
DD$OS_CODE  oi_21[] = { 
  {3,75},
  {4,76},
  {5,77},
  {7,78},
  {8,80} };
DD$OS_CODE  oi_22[] = { 
  {1,36},
  {2,37},
  {12,44} };
DD$OS_CODE  oi_23[] = { 
  {1,79} };
DD$OS_CODE  oi_24[] = { 
  {1,108},
  {2,109},
  {3,110},
  {4,111} };
DD$OS_CODE  oi_25[] = { 
  {4,1},
  {1,2},
  {2,3},
  {7,4},
  {8,5},
  {6,6},
  {5,7},
  {3,64} };
DD$OS_CODE  oi_26[] = { 
  {3,8},
  {4,9} };
DD$OS_CODE  oi_27[] = { 
  {1,10},
  {2,11},
  {3,12} };
DD$OS_CODE  oi_28[] = { 
  {1,13},
  {2,14} };
DD$OS_CODE  oi_29[] = { 
  {1,15},
  {2,16} };
DD$OS_CODE  oi_30[] = { 
  {1,56},
  {2,57} };
DD$OS_CODE  oi_31[] = { 
  {1,17},
  {2,18},
  {3,19} };
DD$OS_CODE  oi_32[] = { 
  {1,20},
  {2,21},
  {3,22},
  {4,23},
  {5,24},
  {6,25},
  {7,26},
  {8,27},
  {9,28},
  {10,29},
  {11,30},
  {13,31},
  {14,32} };
DD$OS_CODE  oi_33[] = { 
  {1,33},
  {2,34},
  {3,35},
  {4,36},
  {5,58},
  {6,59} };
DD$OS_CODE  oi_165[] = { 
  {1,105},
  {2,106},
  {3,107} };
DD$OS_CODE  oi_172[] = { 
  {1,108},
  {2,109},
  {3,110},
  {4,111} };
DD$OS_CODE  oi_281[] = { 
  {0,43},
  {1,44},
  {2,45},
  {3,46},
  {4,47},
  {9,48},
  {5,49},
  {6,50},
  {7,51},
  {8,52} };
DD$OS_CODE  oi_282[] = { 
  {0,37},
  {1,38},
  {2,39},
  {3,40},
  {4,41} };
DD$OS_CODE  oi_283[] = { 
  {1,118},
  {2,119},
  {3,120},
  {5,121},
  {7,122},
  {6,123},
  {8,124},
  {9,144} };
DD$OS_CODE  oi_284[] = { 
  {104,8} };
DD$OS_CODE  oi_285[] = { 
  {1,10},
  {2,10},
  {3,11},
  {5,15},
  {2,16},
  {3,17},
  {6,60},
  {1,62},
  {11,104},
  {16,113},
  {15,114} };
DD$OS_CODE  oi_286[] = { 
  {1,56},
  {2,57},
  {3,53},
  {4,54},
  {5,55} };
DD$OS_CODE  oi_304[] = { 
  {0,59},
  {6,60},
  {1,126},
  {2,127},
  {3,128},
  {5,129},
  {7,130},
  {13,131},
  {14,132},
  {15,133},
  {16,134},
  {18,135},
  {19,136} };
DD$OS_CODE  oi_306[] = { 
  {4,125} };
DD$OS_CODE  oi_307[] = { 
  {5,15},
  {2,16},
  {3,17},
  {11,104} };
DD$OS_CODE  oi_325[] = { 
  {1,10},
  {2,10},
  {3,11},
  {4,155},
  {5,156} };
DD$OS_CODE  oi_337[] = { 
  {2,59},
  {6,60},
  {3,126},
  {10,127},
  {9,128},
  {5,129},
  {7,130},
  {13,131},
  {14,132},
  {15,133},
  {16,134},
  {18,135},
  {19,136},
  {1,137},
  {4,138},
  {8,139},
  {12,140},
  {32,141},
  {64,142},
  {248,143} };
DD$OS_CODE  oi_338[] = { 
  {1,8},
  {2,8},
  {3,8},
  {4,8},
  {5,8},
  {6,8},
  {7,8},
  {8,8},
  {9,8},
  {10,8},
  {12,8},
  {13,8},
  {14,8},
  {15,8},
  {16,8},
  {18,8},
  {19,8},
  {32,8},
  {64,8},
  {248,8} };
DD$OS_CODE  oi_345[] = { 
  {6,146},
  {8,147},
  {9,148},
  {10,149} };
DD$OS_CODE  oi_346[] = { 
  {6,146},
  {8,147},
  {9,148},
  {10,149} };
DD$OS_CODE  oi_347[] = { 
  {6,151},
  {8,152},
  {9,153},
  {10,154} };
DD$OS_CODE  oi_348[] = { 
  {11,6},
  {5,7},
  {6,60},
  {8,61},
  {9,62},
  {10,63} };

/***************** OS_ITEM_STRUCT *********************/

DD$OS_ITEM_DSD  os_item_dsd_struct[] = { 
  {0,0,0,0},
  {1,1,0,0},
  {2,8,0,0},
  {3,2,0,0},
  {4,15,12,0},
  {5,4,22,oi_5},
  {6,4,4,oi_6},
  {7,4,2,oi_7},
  {8,14,19,oi_8},
  {9,14,7,oi_9},
  {10,14,6,oi_10},
  {11,14,6,oi_11},
  {12,4,1,oi_12},
  {13,4,2,oi_13},
  {14,4,2,oi_14},
  {15,14,8,oi_15},
  {16,14,2,oi_16},
  {17,14,3,oi_17},
  {18,14,18,oi_18},
  {19,14,6,oi_19},
  {20,14,4,oi_20},
  {21,14,5,oi_21},
  {22,14,3,oi_22},
  {23,14,1,oi_23},
  {24,14,4,oi_24},
  {25,14,8,oi_25},
  {26,14,2,oi_26},
  {27,14,3,oi_27},
  {28,14,2,oi_28},
  {29,14,2,oi_29},
  {30,14,2,oi_30},
  {31,14,3,oi_31},
  {32,14,13,oi_32},
  {33,14,6,oi_33},
  {34,14,0,0},
  {35,1,0,0},
  {36,1,0,0},
  {37,2,0,0},
  {38,15,256,0},
  {39,2,0,0},
  {40,7,0,0},
  {41,7,0,0},
  {42,2,0,0},
  {43,7,0,0},
  {44,7,0,0},
  {45,7,0,0},
  {46,7,0,0},
  {47,2,0,0},
  {48,2,0,0},
  {49,2,0,0},
  {50,2,0,0},
  {51,2,0,0},
  {52,2,0,0},
  {53,2,0,0},
  {54,7,0,0},
  {55,7,0,0},
  {56,7,0,0},
  {57,7,0,0},
  {58,7,0,0},
  {59,2,0,0},
  {60,2,0,0},
  {61,7,0,0},
  {62,7,0,0},
  {63,2,0,0},
  {64,7,0,0},
  {65,7,0,0},
  {66,2,0,0},
  {67,2,0,0},
  {68,2,0,0},
  {69,7,0,0},
  {70,7,0,0},
  {71,2,0,0},
  {72,2,0,0},
  {73,7,0,0},
  {74,2,0,0},
  {75,7,0,0},
  {76,7,0,0},
  {77,2,0,0},
  {78,7,0,0},
  {79,7,0,0},
  {80,7,0,0},
  {81,7,0,0},
  {82,7,0,0},
  {83,7,0,0},
  {84,2,0,0},
  {85,7,0,0},
  {86,7,0,0},
  {87,2,0,0},
  {88,2,0,0},
  {89,7,0,0},
  {90,7,0,0},
  {91,7,0,0},
  {92,7,0,0},
  {93,2,0,0},
  {94,2,0,0},
  {95,2,0,0},
  {96,2,0,0},
  {97,2,0,0},
  {98,7,0,0},
  {99,2,0,0},
  {100,2,0,0},
  {101,7,0,0},
  {102,2,0,0},
  {103,7,0,0},
  {104,7,0,0},
  {105,7,0,0},
  {106,2,0,0},
  {107,2,0,0},
  {108,2,0,0},
  {109,2,0,0},
  {110,2,0,0},
  {111,2,0,0},
  {112,2,0,0},
  {113,2,0,0},
  {114,2,0,0},
  {115,7,0,0},
  {116,7,0,0},
  {117,7,0,0},
  {118,7,0,0},
  {119,7,0,0},
  {120,2,0,0},
  {121,7,0,0},
  {122,7,0,0},
  {123,7,0,0},
  {124,7,0,0},
  {125,7,0,0},
  {126,7,0,0},
  {127,7,0,0},
  {128,2,0,0},
  {129,2,0,0},
  {130,13,0,0},
  {131,1,0,0},
  {132,2,0,0},
  {133,15,64,0},
  {134,2,0,0},
  {135,2,0,0},
  {136,2,0,0},
  {137,2,0,0},
  {138,2,0,0},
  {139,2,0,0},
  {140,2,0,0},
  {141,2,0,0},
  {142,2,0,0},
  {143,2,0,0},
  {144,2,0,0},
  {145,2,0,0},
  {146,2,0,0},
  {147,2,0,0},
  {148,2,0,0},
  {149,2,0,0},
  {150,2,0,0},
  {151,2,0,0},
  {152,2,0,0},
  {153,2,0,0},
  {154,1,0,0},
  {155,13,0,0},
  {156,13,0,0},
  {157,1,0,0},
  {158,1,0,0},
  {159,13,0,0},
  {160,2,0,0},
  {161,2,0,0},
  {162,2,0,0},
  {163,2,0,0},
  {164,2,0,0},
  {165,14,3,oi_165},
  {166,2,0,0},
  {167,7,0,0},
  {168,7,0,0},
  {169,7,0,0},
  {170,7,0,0},
  {171,2,0,0},
  {172,14,4,oi_172},
  {173,7,0,0},
  {174,7,0,0},
  {175,1,0,0},
  {176,1,0,0},
  {177,2,0,0},
  {178,2,0,0},
  {179,1,0,0},
  {180,1,0,0},
  {181,1,0,0},
  {182,1,0,0},
  {183,2,0,0},
  {184,1,0,0},
  {185,1,0,0},
  {186,13,0,0},
  {187,13,0,0},
  {188,6,0,0},
  {189,6,0,0},
  {190,9,6,0},
  {191,6,0,0},
  {192,13,0,0},
  {193,13,0,0},
  {194,1,0,0},
  {195,9,6,0},
  {196,2,0,0},
  {197,6,0,0},
  {198,13,0,0},
  {199,13,0,0},
  {200,1,0,0},
  {201,6,0,0},
  {202,6,0,0},
  {203,2,0,0},
  {204,2,0,0},
  {205,13,0,0},
  {206,13,0,0},
  {207,13,0,0},
  {208,13,0,0},
  {209,2,0,0},
  {210,13,0,0},
  {211,9,2,0},
  {212,13,0,0},
  {213,9,2,0},
  {214,13,0,0},
  {215,13,0,0},
  {216,1,0,0},
  {217,13,0,0},
  {218,13,0,0},
  {219,13,0,0},
  {220,6,0,0},
  {221,6,0,0},
  {222,6,0,0},
  {223,2,0,0},
  {224,13,0,0},
  {225,13,0,0},
  {226,7,0,0},
  {227,2,0,0},
  {228,2,0,0},
  {229,2,0,0},
  {230,2,0,0},
  {231,2,0,0},
  {232,2,0,0},
  {233,2,0,0},
  {234,2,0,0},
  {235,2,0,0},
  {236,2,0,0},
  {237,2,0,0},
  {238,2,0,0},
  {239,2,0,0},
  {240,2,0,0},
  {241,2,0,0},
  {242,7,0,0},
  {243,6,0,0},
  {244,6,0,0},
  {245,6,0,0},
  {246,6,0,0},
  {247,2,0,0},
  {248,2,0,0},
  {249,2,0,0},
  {250,2,0,0},
  {251,2,0,0},
  {252,2,0,0},
  {253,2,0,0},
  {254,2,0,0},
  {255,2,0,0},
  {256,2,0,0},
  {257,2,0,0},
  {258,2,0,0},
  {259,2,0,0},
  {260,2,0,0},
  {261,2,0,0},
  {262,2,0,0},
  {263,2,0,0},
  {264,2,0,0},
  {265,2,0,0},
  {266,2,0,0},
  {267,2,0,0},
  {268,2,0,0},
  {269,2,0,0},
  {270,2,0,0},
  {271,2,0,0},
  {272,2,0,0},
  {273,2,0,0},
  {274,2,0,0},
  {275,1,0,0},
  {276,1,0,0},
  {277,13,0,0},
  {278,13,0,0},
  {279,1,0,0},
  {280,1,0,0},
  {281,14,10,oi_281},
  {282,5,5,oi_282},
  {283,14,8,oi_283},
  {284,4,1,oi_284},
  {285,14,11,oi_285},
  {286,14,5,oi_286},
  {287,2,0,0},
  {288,7,0,0},
  {289,7,0,0},
  {290,7,0,0},
  {291,7,0,0},
  {292,7,0,0},
  {293,7,0,0},
  {294,15,8,0},
  {295,13,0,0},
  {296,13,0,0},
  {297,13,0,0},
  {298,13,0,0},
  {299,1,0,0},
  {300,13,0,0},
  {301,13,0,0},
  {302,15,6,0},
  {303,9,6,0},
  {304,14,13,oi_304},
  {305,7,0,0},
  {306,14,1,oi_306},
  {307,14,4,oi_307},
  {308,7,0,0},
  {309,1,0,0},
  {310,1,0,0},
  {311,7,0,0},
  {312,7,0,0},
  {313,7,0,0},
  {314,2,0,0},
  {315,7,0,0},
  {316,7,0,0},
  {317,2,0,0},
  {318,2,0,0},
  {319,2,0,0},
  {320,9,6,0},
  {321,9,6,0},
  {322,9,6,0},
  {323,15,8,0},
  {324,9,6,0},
  {325,14,5,oi_325},
  {326,7,0,0},
  {327,2,0,0},
  {328,7,0,0},
  {329,2,0,0},
  {330,7,0,0},
  {331,15,1400,0},
  {332,12,128,0},
  {333,12,128,0},
  {334,18,256,0},
  {335,18,16,0},
  {336,18,16,0},
  {337,14,20,oi_337},
  {338,14,20,oi_338},
  {339,7,0,0},
  {340,7,0,0},
  {341,2,0,0},
  {342,18,10,0},
  {343,13,0,0},
  {344,13,0,0},
  {345,14,4,oi_345},
  {346,14,4,oi_346},
  {347,14,4,oi_347},
  {348,14,6,oi_348},
  {349,1,0,0},
  {350,18,20,0},
  {351,1,0,0},
  {352,18,20,0},
  {353,18,20,0} };

/**************** OS_SEG_ELEMENT_STRUCT **************/

DD$OS_ELEMENT  os_seg_elem_22 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28} };
DD$OS_ELEMENT  os_seg_elem_23 [] = { 
  {-4,4},
  {167,164},
  {168,165},
  {169,166},
  {173,167},
  {174,168} };
DD$OS_ELEMENT  os_seg_elem_24 [] = { 
  {-1,2},
  {-1,2},
  {-1,4},
  {-1,4},
  {-1,1},
  {-1,1},
  {-1,2},
  {-1,12} };
DD$OS_ELEMENT  os_seg_elem_25 [] = { 
  {-1,2},
  {-1,1},
  {-1,1},
  {-1,2},
  {-1,2},
  {-1,4} };
DD$OS_ELEMENT  os_seg_elem_26 [] = { 
  {-1,2},
  {-1,2} };
DD$OS_ELEMENT  os_seg_elem_27 [] = { 
  {-1,4},
  {-1,4},
  {-1,4},
  {-1,4},
  {-1,4} };
DD$OS_ELEMENT  os_seg_elem_28 [] = { 
  {-1,4},
  {-1,4} };
DD$OS_ELEMENT  os_seg_elem_29 [] = { 
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_seg_elem_30 [] = { 
  {183,179},
  {184,180},
  {185,181},
  {186,182},
  {-2,0},
  {281,31},
  {187,183},
  {188,184},
  {-2,0},
  {189,185},
  {190,186},
  {191,187},
  {192,188},
  {193,189} };
DD$OS_ELEMENT  os_seg_elem_32 [] = { 
  {183,179},
  {184,180},
  {185,181},
  {186,182},
  {-2,0},
  {281,31},
  {187,183},
  {220,221},
  {-2,0},
  {221,222},
  {190,186},
  {191,187},
  {192,188},
  {193,189} };
DD$OS_ELEMENT  os_seg_elem_33 [] = { 
  {194,190},
  {195,191},
  {197,193},
  {198,194},
  {199,195} };
DD$OS_ELEMENT  os_seg_elem_34 [] = { 
  {275,273},
  {276,274},
  {277,275},
  {278,276},
  {279,277},
  {280,278} };
DD$OS_ELEMENT  os_seg_elem_35 [] = { 
  {205,208},
  {206,209},
  {207,210},
  {208,211} };
DD$OS_ELEMENT  os_seg_elem_36 [] = { 
  {252,253},
  {253,254},
  {254,255},
  {255,256},
  {256,257},
  {257,258},
  {258,259},
  {259,260},
  {260,261},
  {261,262},
  {262,263},
  {263,264},
  {264,265},
  {265,266},
  {266,267},
  {267,268},
  {268,269},
  {269,270},
  {270,271},
  {271,272} };
DD$OS_ELEMENT  os_seg_elem_37 [] = { 
  {176,172},
  {-1,4},
  {177,173},
  {178,174},
  {179,175},
  {180,176},
  {181,177},
  {182,178} };
DD$OS_ELEMENT  os_seg_elem_39 [] = { 
  {194,190},
  {195,191},
  {222,223},
  {198,194},
  {199,195} };
DD$OS_ELEMENT  os_seg_elem_40 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {12,29},
  {26,31},
  {-2,0},
  {172,30},
  {-1,1},
  {35,33},
  {-1,2},
  {-1,4},
  {-1,2} };
DD$OS_ELEMENT  os_seg_elem_42 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {-4,4},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {7,29},
  {-1,1},
  {283,30},
  {-1,2},
  {-1,2},
  {-1,4},
  {-4,4},
  {157,135},
  {-4,4},
  {158,136},
  {33,31},
  {159,138} };
DD$OS_ELEMENT  os_seg_elem_43 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {-1,1},
  {-1,2},
  {-1,2},
  {-1,4},
  {-1,2},
  {38,36} };
DD$OS_ELEMENT  os_seg_elem_44 [] = { 
  {334,339} };
DD$OS_ELEMENT  os_seg_elem_56 [] = { 
  {-4,4},
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {-4,4},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {13,29},
  {286,31},
  {325,30},
  {35,169},
  {36,33},
  {-1,4},
  {-4,4},
  {167,164},
  {168,165},
  {330,335},
  {326,323} };
DD$OS_ELEMENT  os_seg_elem_57 [] = { 
  {-4,4},
  {-1,2},
  {-1,4},
  {-1,4},
  {-1,4},
  {-1,2},
  {-1,2},
  {-1,2},
  {-1,2},
  {-4,2},
  {-1,2},
  {-1,1},
  {-1,1} };
DD$OS_ELEMENT  os_seg_elem_59 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {-4,4},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {284,29},
  {286,31},
  {307,30},
  {-1,2},
  {36,33},
  {308,293},
  {-4,4},
  {-4,2},
  {309,331},
  {310,332} };
DD$OS_ELEMENT  os_seg_elem_61 [] = { 
  {-4,4},
  {311,294},
  {312,295},
  {313,296},
  {314,333},
  {-1,4},
  {315,297} };
DD$OS_ELEMENT  os_seg_elem_63 [] = { 
  {-4,4},
  {311,325},
  {312,295},
  {313,296},
  {314,333},
  {316,322},
  {315,297} };
DD$OS_ELEMENT  os_seg_elem_64 [] = { 
  {-4,4},
  {-1,4},
  {312,326},
  {313,327},
  {314,333},
  {316,322},
  {315,334} };
DD$OS_ELEMENT  os_seg_elem_65 [] = { 
  {-4,4},
  {317,303},
  {318,304},
  {319,305} };
DD$OS_ELEMENT  os_seg_elem_66 [] = { 
  {-4,1},
  {303,306},
  {320,307},
  {294,308},
  {321,309},
  {322,310},
  {323,311} };
DD$OS_ELEMENT  os_seg_elem_67 [] = { 
  {-4,1},
  {295,312},
  {296,313},
  {297,314},
  {298,315},
  {353,364} };
DD$OS_ELEMENT  os_seg_elem_68 [] = { 
  {-4,1},
  {300,316},
  {301,317},
  {350,361},
  {351,362} };
DD$OS_ELEMENT  os_seg_elem_69 [] = { 
  {324,318},
  {302,319},
  {352,363} };
DD$OS_ELEMENT  os_seg_elem_70 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {-4,4},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {7,29},
  {346,30},
  {-1,1},
  {-1,2},
  {-1,2},
  {-1,4},
  {-4,4},
  {157,135},
  {-4,4},
  {158,136},
  {33,31},
  {159,138} };
DD$OS_ELEMENT  os_seg_elem_72 [] = { 
  {349,360},
  {253,254},
  {254,255},
  {255,256},
  {256,257},
  {257,258},
  {258,259},
  {259,260},
  {260,261},
  {261,262},
  {262,263},
  {263,264},
  {264,265},
  {265,266},
  {266,267},
  {267,268},
  {268,269},
  {269,270},
  {270,271},
  {271,272} };
DD$OS_ELEMENT  os_seg_elem_73 [] = { 
  {183,179},
  {-1,2},
  {185,181},
  {186,182},
  {-2,0},
  {281,31},
  {187,183},
  {188,184},
  {-2,0},
  {189,185},
  {190,186},
  {191,187},
  {192,188},
  {193,189} };

/****************** OS_SEGMENT_STRUCT ****************/

DD$OS_SEGMENT_DSD  os_segment_dsd_struct[] = { 
  {22,8,os_seg_elem_22},
  {23,6,os_seg_elem_23},
  {24,8,os_seg_elem_24},
  {25,6,os_seg_elem_25},
  {26,2,os_seg_elem_26},
  {27,5,os_seg_elem_27},
  {28,2,os_seg_elem_28},
  {29,2,os_seg_elem_29},
  {30,12,os_seg_elem_30},
  {32,12,os_seg_elem_32},
  {33,5,os_seg_elem_33},
  {34,6,os_seg_elem_34},
  {35,4,os_seg_elem_35},
  {36,20,os_seg_elem_36},
  {37,8,os_seg_elem_37},
  {39,5,os_seg_elem_39},
  {40,18,os_seg_elem_40},
  {42,23,os_seg_elem_42},
  {43,17,os_seg_elem_43},
  {44,1,os_seg_elem_44},
  {56,23,os_seg_elem_56},
  {57,13,os_seg_elem_57},
  {59,21,os_seg_elem_59},
  {61,7,os_seg_elem_61},
  {63,7,os_seg_elem_63},
  {64,7,os_seg_elem_64},
  {65,4,os_seg_elem_65},
  {66,7,os_seg_elem_66},
  {67,6,os_seg_elem_67},
  {68,5,os_seg_elem_68},
  {69,3,os_seg_elem_69},
  {70,23,os_seg_elem_70},
  {72,20,os_seg_elem_72},
  {73,12,os_seg_elem_73} };

/************ OS_RECORD_ELEMENT_STRUCT ***************/

DD$OS_ELEMENT  os_rec_elem_250_1 [] = { 
  {-3,22},
  {-4,4},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {-1,1},
  {-1,2},
  {-1,2},
  {-1,4},
  {-1,2},
  {38,36} };
DD$OS_ELEMENT  os_rec_elem_310_1 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27} };
DD$OS_ELEMENT  os_rec_elem_101_278 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {7,29},
  {-1,1},
  {-1,1},
  {-1,2},
  {-1,2},
  {-1,2},
  {157,135},
  {158,136},
  {33,31},
  {159,138},
  {160,139},
  {161,140},
  {162,141},
  {163,142} };
DD$OS_ELEMENT  os_rec_elem_100_4 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {6,29},
  {15,30},
  {-2,0},
  {25,31},
  {-1,1},
  {35,33},
  {-1,2},
  {-1,4},
  {39,45},
  {41,37},
  {42,46},
  {43,38},
  {44,39},
  {45,47},
  {46,40},
  {47,48},
  {48,49},
  {49,50},
  {50,51},
  {51,52},
  {52,53},
  {53,54},
  {54,41},
  {55,42},
  {56,44},
  {57,55},
  {58,56},
  {59,57},
  {60,58},
  {61,59},
  {62,60},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_100_5 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {6,29},
  {15,30},
  {-2,0},
  {25,31},
  {-1,1},
  {35,33},
  {-1,2},
  {-1,4},
  {39,45},
  {40,279},
  {94,94},
  {95,95},
  {96,96},
  {97,97},
  {98,98},
  {99,99},
  {100,100},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_100_6 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {6,29},
  {15,30},
  {-2,0},
  {25,31},
  {-1,1},
  {35,33},
  {-1,2},
  {-1,4},
  {39,45},
  {86,86},
  {87,87},
  {88,88},
  {89,89},
  {90,90},
  {91,91},
  {92,92},
  {93,93},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_100_1 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {6,29},
  {15,30},
  {-2,0},
  {25,31},
  {35,33},
  {-1,2},
  {-1,4},
  {39,45},
  {40,63},
  {65,64},
  {66,65},
  {67,66},
  {68,67},
  {69,68},
  {70,69},
  {71,70},
  {72,71},
  {73,72},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_100_2 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {6,29},
  {15,30},
  {-2,0},
  {25,31},
  {35,33},
  {-1,2},
  {-1,4},
  {39,45},
  {40,63},
  {74,73},
  {164,74},
  {75,75},
  {76,76},
  {77,77},
  {78,78},
  {79,79},
  {80,80},
  {81,81},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_100_7 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {6,29},
  {15,30},
  {-2,0},
  {25,31},
  {35,33},
  {-1,2},
  {-1,4},
  {39,45},
  {40,350},
  {82,82},
  {83,83},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_100_8 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {6,29},
  {15,30},
  {-2,0},
  {25,31},
  {35,33},
  {-1,2},
  {-1,4},
  {39,45},
  {40,351},
  {84,84},
  {85,85},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_106_1 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {12,29},
  {24,30},
  {-1,1},
  {35,33},
  {-1,2},
  {282,31},
  {101,101},
  {102,102},
  {103,103},
  {104,104},
  {105,105},
  {335,340},
  {336,341},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_108_1 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {12,29},
  {-1,1},
  {24,30},
  {-1,2},
  {-1,2},
  {-1,4},
  {101,101},
  {102,102},
  {103,103},
  {104,104},
  {105,105},
  {335,340},
  {336,341},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_106_2 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {12,29},
  {24,30},
  {-1,1},
  {35,33},
  {-1,2},
  {282,31},
  {106,106},
  {107,107},
  {108,108},
  {109,109},
  {110,110},
  {111,111},
  {112,112},
  {113,113},
  {114,114},
  {115,115},
  {116,116},
  {117,117},
  {118,118},
  {119,119},
  {120,120},
  {121,121},
  {122,122},
  {123,123},
  {335,340},
  {336,341},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_105_1 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {13,29},
  {165,30},
  {-2,0},
  {27,31},
  {-1,1},
  {35,33},
  {-1,2},
  {-1,4},
  {124,124},
  {125,125},
  {126,126},
  {127,127},
  {128,128},
  {129,129},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_107_1 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {28,31},
  {-1,1},
  {-1,2},
  {-1,2},
  {-1,4},
  {130,130},
  {131,131} };
DD$OS_ELEMENT  os_rec_elem_109_1 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {32,31},
  {-1,1},
  {-1,2},
  {-1,2},
  {-1,4},
  {132,132},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_300_1 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {-1,1},
  {-1,2},
  {-1,2},
  {-1,4},
  {-1,2},
  {331,36} };
DD$OS_ELEMENT  os_rec_elem_301_1 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {-1,1},
  {-1,2},
  {-1,2},
  {-1,4},
  {-1,2},
  {331,36} };
DD$OS_ELEMENT  os_rec_elem_251_1 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {-1,1},
  {-1,2},
  {-1,2},
  {-1,4},
  {-1,2},
  {38,36} };
DD$OS_ELEMENT  os_rec_elem_252_1 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {-1,1},
  {-1,2},
  {-1,2},
  {-1,4},
  {-1,2},
  {38,36} };
DD$OS_ELEMENT  os_rec_elem_200_1 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {-1,1},
  {-1,2},
  {-1,2},
  {-1,4},
  {133,133},
  {134,134},
  {135,143},
  {136,144},
  {137,61},
  {138,145},
  {139,146},
  {140,147},
  {141,148},
  {142,149},
  {143,152},
  {144,151},
  {145,154},
  {146,155},
  {147,156},
  {148,157},
  {149,158},
  {150,159},
  {151,160},
  {152,161},
  {-4,4},
  {332,337},
  {-4,4},
  {333,338} };
DD$OS_ELEMENT  os_rec_elem_106_3 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {26,31},
  {-2,0},
  {172,30},
  {35,33},
  {-1,2},
  {-1,4},
  {175,169},
  {-1,2},
  {-3,23},
  {-3,23},
  {-3,23},
  {-3,23},
  {-3,23},
  {-3,23},
  {-3,23},
  {-3,23},
  {-3,23},
  {-3,23},
  {-3,23},
  {-3,23},
  {-3,23},
  {-3,23},
  {-3,23},
  {-3,23},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_105_2 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {13,29},
  {27,31},
  {-2,0},
  {165,30},
  {-1,1},
  {35,33},
  {36,32},
  {-1,4},
  {167,164},
  {168,165},
  {169,166},
  {170,170},
  {171,171},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_106_101 [] = { 
  {-3,40},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_106_102 [] = { 
  {-3,40},
  {-3,27},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_106_103 [] = { 
  {-3,40},
  {-3,27},
  {-3,27},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_106_104 [] = { 
  {-3,40},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_106_105 [] = { 
  {-3,40},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_106_106 [] = { 
  {-3,40},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_106_107 [] = { 
  {-3,40},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_106_108 [] = { 
  {-3,40},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_106_109 [] = { 
  {-3,40},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_106_110 [] = { 
  {-3,40},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_106_111 [] = { 
  {-3,40},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_106_112 [] = { 
  {-3,40},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_106_113 [] = { 
  {-3,40},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_106_114 [] = { 
  {-3,40},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_106_115 [] = { 
  {-3,40},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_106_116 [] = { 
  {-3,40},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,27},
  {-3,23},
  {-3,29} };
DD$OS_ELEMENT  os_rec_elem_104_8 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {337,30},
  {-2,0},
  {338,29},
  {35,32},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,73},
  {-1,2},
  {196,192},
  {-3,36} };
DD$OS_ELEMENT  os_rec_elem_103_1 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {20,30},
  {-2,0},
  {10,29},
  {35,32},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,32},
  {-1,2},
  {196,192} };
DD$OS_ELEMENT  os_rec_elem_102_2 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {18,30},
  {-2,0},
  {8,29},
  {-1,2},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,30},
  {-3,33},
  {201,197},
  {203,199},
  {204,200},
  {-3,34} };
DD$OS_ELEMENT  os_rec_elem_103_2 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {20,30},
  {-2,0},
  {10,29},
  {-1,2},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,32},
  {-3,39},
  {201,197},
  {223,224},
  {-1,8},
  {-3,34} };
DD$OS_ELEMENT  os_rec_elem_102_3 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {18,30},
  {-2,0},
  {8,29},
  {35,32},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,30},
  {-3,33},
  {-1,2},
  {203,199},
  {204,200},
  {-3,35},
  {210,212},
  {211,213},
  {212,214},
  {213,215},
  {214,216},
  {215,220} };
DD$OS_ELEMENT  os_rec_elem_102_4 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {18,30},
  {-2,0},
  {8,29},
  {35,32},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,30},
  {-3,33},
  {-1,2},
  {203,199},
  {204,200},
  {-3,35},
  {210,212},
  {217,217},
  {218,218},
  {213,215},
  {214,216},
  {219,219},
  {215,220} };
DD$OS_ELEMENT  os_rec_elem_102_5 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {18,30},
  {-2,0},
  {8,29},
  {35,32},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,30},
  {-3,33},
  {200,196},
  {203,199},
  {-3,36} };
DD$OS_ELEMENT  os_rec_elem_102_6 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {18,30},
  {-2,0},
  {8,29},
  {35,32},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,30},
  {-3,33},
  {201,197},
  {203,199},
  {204,200},
  {-3,34} };
DD$OS_ELEMENT  os_rec_elem_102_7 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {18,30},
  {-2,0},
  {8,29},
  {35,32},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,30},
  {-3,33},
  {202,198},
  {203,199},
  {274,202},
  {272,204},
  {273,206},
  {216,207} };
DD$OS_ELEMENT  os_rec_elem_103_3 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {20,30},
  {-2,0},
  {10,29},
  {35,32},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,32},
  {-3,39},
  {201,197},
  {223,224},
  {224,225},
  {225,226},
  {-1,2},
  {-3,36} };
DD$OS_ELEMENT  os_rec_elem_103_4 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {20,30},
  {-2,0},
  {10,29},
  {35,32},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,32},
  {-3,39},
  {-1,2},
  {223,224},
  {224,225},
  {225,226},
  {-1,2},
  {242,243},
  {243,244},
  {244,245},
  {245,246},
  {246,247} };
DD$OS_ELEMENT  os_rec_elem_103_5 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {20,30},
  {-2,0},
  {10,29},
  {35,32},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,32},
  {-3,39},
  {-1,2},
  {223,224},
  {224,225},
  {225,226},
  {-1,2},
  {226,227},
  {227,228},
  {228,229},
  {229,230},
  {230,231},
  {231,232},
  {232,233},
  {233,234},
  {234,235},
  {235,236},
  {236,237},
  {237,238},
  {238,239},
  {239,240},
  {240,241},
  {241,242} };
DD$OS_ELEMENT  os_rec_elem_103_6 [] = { 
  {-3,22},
  {154,27},
  {-2,0},
  {5,27},
  {-1,1},
  {20,30},
  {-2,0},
  {10,29},
  {35,32},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,32},
  {-3,39},
  {-1,2},
  {223,224},
  {224,225},
  {225,226},
  {-1,2},
  {247,248},
  {248,249},
  {249,250},
  {250,251},
  {251,252} };
DD$OS_ELEMENT  os_rec_elem_101_1 [] = { 
  {-3,42},
  {160,280},
  {161,281},
  {162,282} };
DD$OS_ELEMENT  os_rec_elem_101_2 [] = { 
  {-3,42},
  {160,139},
  {161,140},
  {162,141},
  {163,142} };
DD$OS_ELEMENT  os_rec_elem_101_3 [] = { 
  {-3,42},
  {160,283},
  {161,284},
  {162,285} };
DD$OS_ELEMENT  os_rec_elem_101_5 [] = { 
  {-3,42},
  {56,44},
  {59,57},
  {54,41},
  {55,42} };
DD$OS_ELEMENT  os_rec_elem_101_7 [] = { 
  {-3,42},
  {160,286},
  {161,287},
  {162,288},
  {163,289} };
DD$OS_ELEMENT  os_rec_elem_101_6 [] = { 
  {-3,42},
  {160,290},
  {161,291} };
DD$OS_ELEMENT  os_rec_elem_350_1 [] = { 
  {-3,43} };
DD$OS_ELEMENT  os_rec_elem_351_1 [] = { 
  {-3,43} };
DD$OS_ELEMENT  os_rec_elem_104_4 [] = { 
  {-3,22},
  {-4,4},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {13,29},
  {306,30},
  {-2,0},
  {286,31},
  {-1,1},
  {35,169},
  {36,33},
  {-1,4},
  {-4,4},
  {167,164},
  {168,165},
  {169,166},
  {288,292},
  {-1,4},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_106_4 [] = { 
  {-3,22},
  {-4,4},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {12,29},
  {172,30},
  {-2,0},
  {26,31},
  {-1,1},
  {35,33},
  {-1,2},
  {-1,4},
  {-4,4},
  {92,92},
  {93,93},
  {160,286},
  {289,299},
  {290,300},
  {334,339} };
DD$OS_ELEMENT  os_rec_elem_105_4 [] = { 
  {-3,22},
  {-4,4},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {13,29},
  {165,30},
  {-2,0},
  {27,31},
  {-1,1},
  {35,33},
  {-1,2},
  {-1,4},
  {-4,4},
  {291,299},
  {292,301},
  {169,166},
  {293,302} };
DD$OS_ELEMENT  os_rec_elem_104_1001 [] = { 
  {-3,56},
  {169,166},
  {327,320} };
DD$OS_ELEMENT  os_rec_elem_104_1002 [] = { 
  {-3,56},
  {328,324},
  {329,321} };
DD$OS_ELEMENT  os_rec_elem_104_101 [] = { 
  {-3,59} };
DD$OS_ELEMENT  os_rec_elem_104_102 [] = { 
  {-3,59},
  {-3,61} };
DD$OS_ELEMENT  os_rec_elem_104_104 [] = { 
  {-3,59},
  {-4,4},
  {-1,4},
  {-1,4},
  {-1,4},
  {-1,4},
  {-1,4},
  {-1,4},
  {-4,4},
  {-3,23} };
DD$OS_ELEMENT  os_rec_elem_104_105 [] = { 
  {-3,59},
  {-3,63},
  {-4,4},
  {-3,23} };
DD$OS_ELEMENT  os_rec_elem_104_106 [] = { 
  {-3,59},
  {-3,64},
  {-4,4},
  {-3,23} };
DD$OS_ELEMENT  os_rec_elem_104_107 [] = { 
  {-3,59},
  {-3,61},
  {-4,4},
  {-1,4},
  {-1,4},
  {-1,4},
  {-1,4},
  {-3,65} };
DD$OS_ELEMENT  os_rec_elem_104_108 [] = { 
  {-3,59},
  {-3,63},
  {-4,4},
  {-3,23},
  {-3,65} };
DD$OS_ELEMENT  os_rec_elem_104_109 [] = { 
  {-3,59},
  {-3,64},
  {-4,4},
  {-3,23},
  {-3,65} };
DD$OS_ELEMENT  os_rec_elem_104_110 [] = { 
  {-3,59},
  {-3,66} };
DD$OS_ELEMENT  os_rec_elem_104_111 [] = { 
  {-3,59},
  {-3,66},
  {-3,68} };
DD$OS_ELEMENT  os_rec_elem_104_112 [] = { 
  {-3,59},
  {-3,66},
  {-3,69} };
DD$OS_ELEMENT  os_rec_elem_104_113 [] = { 
  {-3,59},
  {-3,66},
  {-3,67} };
DD$OS_ELEMENT  os_rec_elem_104_3 [] = { 
  {-3,22},
  {-4,4},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {13,29},
  {304,30},
  {-2,0},
  {286,31},
  {-1,1},
  {35,329},
  {36,33},
  {-1,4},
  {-4,4},
  {305,330} };
DD$OS_ELEMENT  os_rec_elem_101_8 [] = { 
  {-3,42},
  {160,342},
  {161,343},
  {162,344} };
DD$OS_ELEMENT  os_rec_elem_104_6 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {337,30},
  {-2,0},
  {338,29},
  {35,329},
  {36,32},
  {-1,4},
  {-3,57},
  {-3,73},
  {-3,72} };
DD$OS_ELEMENT  os_rec_elem_104_7 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,29},
  {-1,1},
  {337,30},
  {-2,0},
  {338,29},
  {35,329},
  {36,32},
  {-1,4},
  {-3,57},
  {-3,32},
  {-3,39},
  {201,197},
  {223,224},
  {-1,8},
  {-3,34} };
DD$OS_ELEMENT  os_rec_elem_104_2 [] = { 
  {-3,22},
  {-4,4},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {13,29},
  {286,31},
  {304,30},
  {35,329},
  {36,33},
  {-1,4},
  {-4,4},
  {305,347} };
DD$OS_ELEMENT  os_rec_elem_104_18 [] = { 
  {-3,22},
  {-4,4},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {13,29},
  {286,31},
  {304,30},
  {35,329},
  {36,33},
  {-1,4},
  {-4,4},
  {305,348} };
DD$OS_ELEMENT  os_rec_elem_100_3 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {6,29},
  {15,30},
  {-2,0},
  {25,31},
  {35,33},
  {-1,2},
  {-1,4},
  {39,45},
  {40,352},
  {82,82},
  {83,83},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_101_9 [] = { 
  {-3,42},
  {160,342},
  {161,57} };
DD$OS_ELEMENT  os_rec_elem_101_4 [] = { 
  {-3,42},
  {160,283},
  {161,284},
  {162,285} };
DD$OS_ELEMENT  os_rec_elem_102_8 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {18,30},
  {-2,0},
  {8,29},
  {35,32},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,30},
  {-3,33},
  {-1,2},
  {203,199},
  {204,200},
  {-3,35},
  {210,212},
  {217,217},
  {343,353},
  {213,215},
  {214,216},
  {219,219},
  {215,220} };
DD$OS_ELEMENT  os_rec_elem_102_9 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {18,30},
  {-2,0},
  {8,29},
  {35,32},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,30},
  {-3,33},
  {-1,2},
  {203,199},
  {204,200},
  {-3,35},
  {210,212},
  {217,217},
  {344,354},
  {213,215},
  {214,216},
  {219,219},
  {215,220} };
DD$OS_ELEMENT  os_rec_elem_102_10 [] = { 
  {-3,22},
  {154,21},
  {-2,0},
  {5,27},
  {-1,1},
  {18,30},
  {-2,0},
  {8,29},
  {35,32},
  {36,33},
  {-1,4},
  {-3,57},
  {-3,30},
  {-3,33},
  {-1,2},
  {203,199},
  {204,200},
  {-3,35},
  {210,212},
  {217,217},
  {-1,1},
  {213,215},
  {214,216},
  {219,219},
  {215,220} };
DD$OS_ELEMENT  os_rec_elem_101_10 [] = { 
  {-3,70},
  {160,290},
  {161,291} };
DD$OS_ELEMENT  os_rec_elem_101_11 [] = { 
  {-3,70},
  {160,286},
  {161,287},
  {162,288},
  {163,289} };
DD$OS_ELEMENT  os_rec_elem_101_12 [] = { 
  {-3,70},
  {160,286},
  {161,287},
  {162,288},
  {163,289} };
DD$OS_ELEMENT  os_rec_elem_101_13 [] = { 
  {-3,70},
  {160,286},
  {161,287},
  {162,288},
  {163,289} };
DD$OS_ELEMENT  os_rec_elem_100_9 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {6,29},
  {-1,1},
  {347,30},
  {-2,0},
  {348,31},
  {35,33},
  {-1,2},
  {-1,4},
  {39,45},
  {40,279},
  {94,94},
  {95,95},
  {96,96},
  {97,97},
  {98,98},
  {99,99},
  {100,100},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_100_10 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {6,29},
  {-1,1},
  {347,30},
  {-2,0},
  {348,31},
  {35,33},
  {-1,2},
  {-1,4},
  {39,45},
  {86,86},
  {87,87},
  {88,88},
  {89,89},
  {90,90},
  {91,91},
  {92,92},
  {93,93},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_100_11 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {6,29},
  {-1,1},
  {347,30},
  {-2,0},
  {348,31},
  {35,33},
  {-1,2},
  {-1,4},
  {39,45},
  {86,86},
  {87,87},
  {88,88},
  {89,89},
  {90,90},
  {91,91},
  {92,92},
  {93,93},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_100_12 [] = { 
  {-1,2},
  {1,22},
  {2,24},
  {3,26},
  {-1,1},
  {-1,1},
  {-1,2},
  {4,28},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {6,29},
  {-1,1},
  {347,30},
  {-2,0},
  {348,31},
  {35,33},
  {-1,2},
  {-1,4},
  {39,45},
  {86,86},
  {87,87},
  {88,88},
  {89,89},
  {90,90},
  {91,91},
  {92,92},
  {93,93},
  {63,61},
  {64,62} };
DD$OS_ELEMENT  os_rec_elem_104_1003 [] = { 
  {-3,22},
  {-4,4},
  {154,27},
  {-2,0},
  {5,27},
  {-2,0},
  {13,29},
  {286,31},
  {325,30},
  {35,169},
  {36,33},
  {-1,4},
  {-4,4},
  {168,165},
  {169,166},
  {339,345},
  {340,355},
  {341,356},
  {342,357} };
DD$OS_ELEMENT  os_rec_elem_104_1 [] = { 
  {-3,22},
  {-4,4},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {13,29},
  {286,31},
  {304,30},
  {35,329},
  {36,33},
  {-1,4},
  {-4,4},
  {305,359} };
DD$OS_ELEMENT  os_rec_elem_104_14 [] = { 
  {-3,22},
  {-4,4},
  {154,21},
  {-2,0},
  {5,27},
  {-2,0},
  {13,29},
  {286,31},
  {304,30},
  {35,329},
  {36,33},
  {-1,4},
  {-4,4},
  {305,358} };

/**************** OS_RECORD_STRUCT *******************/

DD$OS_RECORD_DSD  os_record_dsd_struct[] = { 
  {250,1,11,os_rec_elem_250_1},
  {310,1,10,os_rec_elem_310_1},
  {101,278,24,os_rec_elem_101_278},
  {100,4,42,os_rec_elem_100_4},
  {100,5,28,os_rec_elem_100_5},
  {100,6,28,os_rec_elem_100_6},
  {100,1,29,os_rec_elem_100_1},
  {100,2,29,os_rec_elem_100_2},
  {100,7,22,os_rec_elem_100_7},
  {100,8,22,os_rec_elem_100_8},
  {106,1,25,os_rec_elem_106_1},
  {108,1,25,os_rec_elem_108_1},
  {106,2,38,os_rec_elem_106_2},
  {105,1,25,os_rec_elem_105_1},
  {107,1,17,os_rec_elem_107_1},
  {109,1,18,os_rec_elem_109_1},
  {300,1,17,os_rec_elem_300_1},
  {301,1,17,os_rec_elem_301_1},
  {251,1,17,os_rec_elem_251_1},
  {252,1,17,os_rec_elem_252_1},
  {200,1,39,os_rec_elem_200_1},
  {106,3,28,os_rec_elem_106_3},
  {105,2,17,os_rec_elem_105_2},
  {106,101,3,os_rec_elem_106_101},
  {106,102,4,os_rec_elem_106_102},
  {106,103,5,os_rec_elem_106_103},
  {106,104,6,os_rec_elem_106_104},
  {106,105,7,os_rec_elem_106_105},
  {106,106,8,os_rec_elem_106_106},
  {106,107,9,os_rec_elem_106_107},
  {106,108,10,os_rec_elem_106_108},
  {106,109,11,os_rec_elem_106_109},
  {106,110,12,os_rec_elem_106_110},
  {106,111,13,os_rec_elem_106_111},
  {106,112,14,os_rec_elem_106_112},
  {106,113,15,os_rec_elem_106_113},
  {106,114,16,os_rec_elem_106_114},
  {106,115,17,os_rec_elem_106_115},
  {106,116,18,os_rec_elem_106_116},
  {104,8,14,os_rec_elem_104_8},
  {103,1,13,os_rec_elem_103_1},
  {102,2,16,os_rec_elem_102_2},
  {103,2,16,os_rec_elem_103_2},
  {102,3,22,os_rec_elem_102_3},
  {102,4,23,os_rec_elem_102_4},
  {102,5,15,os_rec_elem_102_5},
  {102,6,16,os_rec_elem_102_6},
  {102,7,18,os_rec_elem_102_7},
  {103,3,18,os_rec_elem_103_3},
  {103,4,22,os_rec_elem_103_4},
  {103,5,33,os_rec_elem_103_5},
  {103,6,22,os_rec_elem_103_6},
  {101,1,4,os_rec_elem_101_1},
  {101,2,5,os_rec_elem_101_2},
  {101,3,4,os_rec_elem_101_3},
  {101,5,5,os_rec_elem_101_5},
  {101,7,5,os_rec_elem_101_7},
  {101,6,3,os_rec_elem_101_6},
  {350,1,1,os_rec_elem_350_1},
  {351,1,1,os_rec_elem_351_1},
  {104,4,19,os_rec_elem_104_4},
  {106,4,18,os_rec_elem_106_4},
  {105,4,16,os_rec_elem_105_4},
  {104,1001,3,os_rec_elem_104_1001},
  {104,1002,3,os_rec_elem_104_1002},
  {104,101,1,os_rec_elem_104_101},
  {104,102,2,os_rec_elem_104_102},
  {104,104,10,os_rec_elem_104_104},
  {104,105,4,os_rec_elem_104_105},
  {104,106,4,os_rec_elem_104_106},
  {104,107,8,os_rec_elem_104_107},
  {104,108,5,os_rec_elem_104_108},
  {104,109,5,os_rec_elem_104_109},
  {104,110,2,os_rec_elem_104_110},
  {104,111,3,os_rec_elem_104_111},
  {104,112,3,os_rec_elem_104_112},
  {104,113,3,os_rec_elem_104_113},
  {104,3,13,os_rec_elem_104_3},
  {101,8,4,os_rec_elem_101_8},
  {104,6,12,os_rec_elem_104_6},
  {104,7,16,os_rec_elem_104_7},
  {104,2,12,os_rec_elem_104_2},
  {104,18,12,os_rec_elem_104_18},
  {100,3,22,os_rec_elem_100_3},
  {101,9,3,os_rec_elem_101_9},
  {101,4,4,os_rec_elem_101_4},
  {102,8,23,os_rec_elem_102_8},
  {102,9,23,os_rec_elem_102_9},
  {102,10,23,os_rec_elem_102_10},
  {101,10,3,os_rec_elem_101_10},
  {101,11,5,os_rec_elem_101_11},
  {101,12,5,os_rec_elem_101_12},
  {101,13,5,os_rec_elem_101_13},
  {100,9,28,os_rec_elem_100_9},
  {100,10,28,os_rec_elem_100_10},
  {100,11,28,os_rec_elem_100_11},
  {100,12,28,os_rec_elem_100_12},
  {104,1003,17,os_rec_elem_104_1003},
  {104,1,12,os_rec_elem_104_1},
  {104,14,12,os_rec_elem_104_14} };

/************  EXTERNAL POINTERS *************/

long std_item_dsd_count = 341;
long std_segment_dsd_count = 132;
long os_item_dsd_count = 353;
long os_segment_dsd_count = 34;
long os_record_dsd_count = 100;

DD$STD_ITEM_DSD_PTR	std_item_dsd_anchor;
DD$STD_SEGMENT_DSD_PTR	std_segment_dsd_anchor;
DD$OS_ITEM_DSD_PTR	os_item_dsd_anchor;
DD$OS_SEGMENT_DSD_PTR	os_segment_dsd_anchor;
DD$OS_RECORD_DSD_PTR	os_record_dsd_anchor;


dsd_init()
{
std_item_dsd_anchor    = std_item_dsd_struct;
std_segment_dsd_anchor = std_seg_dsd_struct;
os_item_dsd_anchor     = os_item_dsd_struct;
os_segment_dsd_anchor  = os_segment_dsd_struct;
os_record_dsd_anchor   = os_record_dsd_struct;
}

