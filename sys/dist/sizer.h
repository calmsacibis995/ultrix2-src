struct uba_ctlr ctlr;
struct uba_device   device;
struct uba_driver   driver;
struct mba_device   mdevice;
struct mba_slave    sdevice;
struct mba_driver   mdriver;
struct rpb	    rpb;
struct config_adpt  adpt;
struct timezone tzb;


struct nlist    nl[] =
{
	{ /* 0 */	"_ubminit" },
	{ /* 1 */	"_ubdinit" },
	{ /* 2 */	"_uba_hd" },
	{ /* 3 */	"_cpu" },
	{ /* 4 */	"_mbdinit" },
	{ /* 5 */	"_mbsinit" },
	{ /* 6 */	"_mba" },
	{ /* 7 */	"_tz" },
	{ /* 8 */	"_rootdev" },
	{ /* 9 */	"_swdevt" },
	{ /* 10 */	"_dumpdev" },
	{ /* 11 */	"_argdev" },
	{ /* 12 */	"_Physmem" },
	{ /* 13 */	"_rpb" },
	{ /* 14 */	"_config_adpt" },
	{ /* 15 */	"_maxcpu" },
	{ /* 16 */	"_umem"	},
	{ /* 17 */	"_cpu_subtype"},
	{ /* 18 */	"_qmem"	},
	{ /* end */	""	}
};

struct strtcputype
{
    int cpuno;
    char *cpustrg;
    int maxusers;
    int maxubas;
    int maxmbas;
    char *bootdev;
} cpu[] =
    {
    VAX_780,	 "VAX780",	  32,	 4,	 4,	"boot780",
    VAX_750,	 "VAX750",	  32,	 2,	 2,	"boot750",
    VAX_730,	 "VAX730",	  16,	 1,	 0,	"boot730",
    VAX_8200,	 "VAX8200",	  32,	 7,	 0,	"boot8200",
    VAX_8600,	 "VAX8600",	 128,	 7,	 4,	"boot8600",
    VAX_8800,	 "VAX8800",	 128,	 7,	 4, 	"boot8800",
    MVAX_I,	 "MVAX",	   8,	 1,	 0,	"bootI",
    MVAX_II,	 "MVAX",	   8,	 1,	 0,	"bootII",
    -1, 	 "**UNDEFINED*..*CPU**", 0, 0, 0,	"\0"
    };
struct a {
	char type[20];
	char ultname[10];
	char link[10];
	char vectors[75];
	int node;
	int sup;	/* 0 if supported; 1 if unsupported */
	int csr;
	char flags[10];
	int drive;
	int makedevflag;
}all[100];

struct nms
{
	char   *uname;		/* Ultrix style name */
	char   *devname;	/* config file name */
	int	makedevflag;	/* 0-nomake 1-make */
	int 	supp;		/* 0-supported 1-unsupported */
} names[] = {

"uq",	"controller",	0,	0,	"ra",	"disk",		1,	0,
"hl",	"controller",	0,	0,	"rl",	"disk",		1,	0,
"hk",	"controller",	0,	0,	"rk",	"disk",		1,	0,
"zs",	"controller",	0,	0,	"ts",	"tape",		1,	0,
"lp",	"device",	1,	0,	"dz",	"device",	1,	0,
"dmz",	"device",	1,	0,	"dhu",	"device",	1,	0,
"dmf",	"device",	1,	0,	"dmc",	"device",	0,	0,
"de",	"device",	0,	0,	"hp",	"disk",		1,	0,
"ht",	"master",	0,	0,	"mt",	"master",	0,	0,
"uba",	"adapter",	0,	0,	"mba",	"controller",	0,	0,
"tu",	"tape",		1,	0,	"mu",	"tape",		1,	0,
"rb",	"disk",		1,	0,	"tm",	"controller",	0,	1,
"te",	"tape",		1,	0,	"ut",	"controller",	0,	1,
"tj",	"tape",		1,	0,	"sc",	"controller",	0,	0,
"up",	"disk",		1,	1,	"idc",	"controller",	0,	0,
"dh",	"device",	1,	1,	"acc",	"device",	0,	1,
"ad",	"device",	1,	1,	"css",	"device",	0,	1,
"ct",	"device",	1,	1,	"dn",	"device",	0,	1,
"dm",	"device",	1,	1,	"ec",	"device",	0,	1,
"en",	"device",	0,	1,	"hy",	"device",	0,	1,
"ik",	"device",	1,	1,	"il",	"device",	0,	1,
"kg",	"device",	1,	1,	"pcl",	"device",	0,	1,
"ps",	"device",	1,	1,	"fx",	"controller",	0,	1,
"rx",	"disk",		1,	0,	"un",	"device",	0,	1,
"uu",	"device",	1,	1,	"va",	"controller",	0,	1,
"vz",	"disk",		1,	1,	"vp",	"device",	1,	1,
"vv",	"device",	1,	1,	"tmscp","controller",	0,	0,
"tms",	"tape",		1,	0,	"rqd",	"controller",	0,	0,
"qe",	"device",	0,	0,	"vaxbi", "adapter",	0,	0,
"kdb",	"controller",	0,	0,	"klesib", "controller",	0,	0,
"aie",	"controller",	0,	0,	"bvpni", "device",	0,	0,
"dmb",	"device",	1,	0,	"klesiu", "controller",	0,	0,
"uda",	"controller",	0,	0,	"sdc",	"controller",	0,	0,
"ss",	"device",	1,	0,	"sh",	"device",	1,	0,
"stc",	"controller",	0,	0,	"st",	"tape",		1,	0,
"se",	"device",	0,	0,	"sm",	"device",	1,	0,
"sg",	"device",	1,	0,	"rd",	"disk",		1,	0,
"qd",	"device",	1,	0,	"qv",	"device",	1,	0,
"dup",	"device",	0,	0,	"dpv",	"device",	0,	0,
"urx",	"disk",		1,	1,	"\0",	"\0",		0,	0
};


static struct iv
{
	char   *devnam;
	char   *ivector;
} intervec[] = {

"acc",	"accrint",	"acc",	"accxint",	"css",	"cssrint",
"css",	"cssxint",	"de",	"deintr",	"dh",	"dhrint",
"dh",	"dhxint",	"dm",	"dmintr",	"dmc",	"dmcrint",
"dmc",	"dmcxint",	"dmf",	"dmfsrint",	"dmf",	"dmfsxint",
"dmf",	"dmfdaint",	"dmf",	"dmfdbint",	"dmf",	"dmfrint",
"dmf",	"dmfxint",	"dmf",	"dmflint",	"dn",	"dnintr",
"dz",	"dzrint",	"dz",	"dzxint",	"dmz",	"dmzrinta",
"dmz",	"dmzxinta",	"dmz",	"dmzrintb",	"dmz",	"dmzxintb",
"dmz",	"dmzrintc",	"dmz",	"dmzxintc",	"ec",	"ecrint",
"ec",	"ecxint",	"ec",	"eccollide",	"en",	"enrint",
"en",	"enxint",	"en",	"encollide",	"hk",	"rkintr",
"hy",	"hyint",	"ik",	"ikintr",	"il",	"ilrint",
"il",	"ilcint",	"lp",	"lpintr",	"pcl",	"pclxint",
"pcl",	"pclrint",	"fx",	"rxintr",	"tm",	"tmintr",
"zs",	"tsintr",	"uq",	"uqintr",	"un",	"unintr",
"sc",	"upintr",	"ut",	"utintr",	"vp",	"vpintr",
"vp",	"vpintr",       "vv",	"vvrint",	"vv",	"vvxint",
"idc",	"idcintr",	"hl",	"rlintr",       "dhu",	"dhurint",
"dhu",	"dhuxint",      "qe",	"qeintr",	"tmscp","tmscpintr",
"dmb",	"dmbsint",	"dmb",	"dmbaint",	"dmb",	"dmblint",	
"qv",	"qvkint",	"qv",	"qvvint",	"qd",	"qddint",
"qd",	"qdaint",	"qd",	"qdiint",	"bvpni","bvpniintr",
"sdc",	"sdintr",	"ss",	"ssrint",	"ss",	"ssxint",
"stc",	"stintr",	"se",	"seintr",	"sm",	"smvint",
"sg",	"sgaint",	"sg",	"sgfint",	"sh",	"shrint",
"sh",	"shxint",	"dup",	"duprint",	"dup",	"dupxint",
"dpv",	"dpvrint",	"dpv",	"dpvxint",	"",	""
};

struct d {
	char name[20];
	int gap;
} devs[] = 
{
	"dj11",	 04,	"dh",	 010,	"dq11",	 04,	"du11",	 04,
	"dup",	 04,	"lk11",	 04,	"dmc",	 04,	"dz",	 04,
	"kmc11", 04,	"lpp11", 04,	"vmv21", 04,	"vmv31", 010,
	"dwr70", 04,	"hl",	 04,	"lpa11", 010,	"kw11c", 04,
	"rsv",	 04,	"fx",    04,	"un",	 04,	"hy",    04,
	"dmp11", 04,	"dpv",   04,	"isb11", 04,	"dmv11", 010,
	"de",	 04,	"uda",	 02,	"dmf",   020,	"kms11", 010,
	"vs100", 010,	"klesiu",02,	"kmv11", 010,	"dhu",   010,
	"dmz",   020,	"cpi32", 020,	"qvss",	 040,	"vs31",	 04,
	"dtqna", 04,	"csam",	 04,	"adv11c",04,	"aav11c",04,
	"axv11c",04,	"kwv11c",02,	"adv11d",04,	"aav11d",04,
	"drq3p", 04,	"\0", 	 0
};

struct h {
	char type[20];
} hardwr[] = {"adapter", "controller", "disk", "tape", "master", "device",""};

struct f {
	char name[10];
	char flags[20];
} fl[] = 
{
	"css",	"10",		"dh",	"0xffff",	"dm",	"0xffff",
	"dhu",	"0xffff",	"dmf",	"0xff",		"dmz",	"0xffffff",
	"dmb",	"0xff",		"qv",	"0x0f",		"qd",	"0x0f",
	"sm",	"0x0f",		"sg",	"0x0f",		"dz",	"0xff",
	"sh",	"0xff",		"ss",	"0x0f",		"\0",	"\0"
};
