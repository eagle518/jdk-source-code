/*
 * @(#)ipv6_defs.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifndef IPV6_DEFS_H
#define IPV6_DEFS_H

/* from socket.h */
#define AF_INET6	26		/* Internet Protocol, Version 6 */
#define PF_INET6        AF_INET6
/* from in/netdb.h */
#define IPPROTO_IPV6	41              /* IPv6 encapsulated in IP */

/*
 * IPv6 options
 */
#define IPV6_UNICAST_HOPS	0x5	/* hop limit value for unicast */
					/* packets. */
					/* argument type: uint_t */
#define IPV6_MULTICAST_IF	0x6	/* outgoing interface for */
					/* multicast packets. */
					/* argument type: struct in6_addr */
#define IPV6_MULTICAST_HOPS	0x7	/* hop limit value to use for */
					/* multicast packets. */
					/* argument type: uint_t */
#define IPV6_MULTICAST_LOOP	0x8	/* enable/disable delivery of */
					/* multicast packets on same socket. */
					/* argument type: uint_t */
#define IPV6_JOIN_GROUP		0x9	/* join an IPv6 multicast group. */
					/* argument type: struct ipv6_mreq */
#define IPV6_LEAVE_GROUP	0xa	/* leave an IPv6 multicast group */
					/* argument type: struct ipv6_mreq */
/*
 * IPV6_ADD_MEMBERSHIP and IPV6_DROP_MEMBERSHIP are being kept
 * for backward compatibility. They have the same meaning as IPV6_JOIN_GROUP
 * and IPV6_LEAVE_GROUP respectively.
 */
#define IPV6_ADD_MEMBERSHIP	0x9	 /* join an IPv6 multicast group. */
					/* argument type: struct ipv6_mreq */
#define IPV6_DROP_MEMBERSHIP	0xa	/* leave an IPv6 multicast group */
					/* argument type: struct ipv6_mreq */


/* addrinfo flags */
#define AI_PASSIVE      0x0008  /* intended for bind() + listen() */
#define AI_CANONNAME    0x0010  /* return canonical version of host */
#define AI_NUMERICHOST  0x0020  /* use numeric node address string */

/* getnameinfo max sizes as defined in spec */

#define NI_MAXHOST      1025
#define NI_MAXSERV      32

/* getnameinfo flags */

#define NI_NOFQDN       0x0001
#define NI_NUMERICHOST  0x0002  /* return numeric form of address */
#define NI_NAMEREQD     0x0004  /* request DNS name */
#define NI_NUMERICSERV  0x0008
#define NI_DGRAM        0x0010



struct in6_addr {
  union {
    /*
     * Note: Static initalizers of "union" type assume
     * the constant on the RHS is the type of the first member
     * of union.
     * To make static initializers (and efficient usage) work,
     * the order of members exposed to user and kernel view of
     * this data structure is different.
     * User environment sees specified uint8_t type as first
     * member whereas kernel sees most efficient type as
     * first member.
     */
#ifdef _KERNEL
    uint32_t        _S6_u32[4];     /* IPv6 address */
    uint8_t         _S6_u8[16];     /* IPv6 address */
#else
    uint8_t         _S6_u8[16];     /* IPv6 address */
    uint32_t        _S6_u32[4];     /* IPv6 address */
#endif
    uint32_t        __S6_align;     /* Align on 32 bit boundary */
  } _S6_un;
};
#define s6_addr         _S6_un._S6_u8

/*
 * IPv6 socket address.
 */
struct sockaddr_in6 {
  sa_family_t     sin6_family;
  in_port_t       sin6_port;
  uint32_t        sin6_flowinfo;
  struct in6_addr sin6_addr;
  uint32_t        sin6_scope_id;  /* Depends on scope of sin6_addr */
  uint32_t        __sin6_src_id;  /* Impl. specific - UDP replies */
};


/*
 * Argument structure for IPV6_JOIN_GROUP and IPV6_LEAVE_GROUP on
 * IPv6 addresses.
 */
struct ipv6_mreq {
  struct in6_addr	ipv6mr_multiaddr;       /* IPv6 multicast addr */
  unsigned int		ipv6mr_interface;       /* interface index */
};


struct addrinfo {
  int ai_flags;		/* AI_PASSIVE, AI_CANONNAME */
  int ai_family;		/* PF_xxx */
  int ai_socktype;	/* SOCK_xxx */
  int ai_protocol;	/* 0 or IPPROTO_xxx for IPv4 and IPv6 */
  size_t ai_addrlen;	/* length of ai_addr */
  char *ai_canonname;	/* canonical name for hostname */
  struct sockaddr *ai_addr;	/* binary address */
  struct addrinfo *ai_next;	/* next structure in linked list */
};

/*
 * sockaddr_storage:
 * Common superset of at least AF_INET, AF_INET6 and AF_LINK sockaddr
 * structures. Has sufficient size and alignment for those sockaddrs.
 */

/*
 * Desired maximum size, alignment size and related types.
 */
#define _SS_MAXSIZE     256     /* Implementation specific max size */

/*
 * To represent desired sockaddr max alignment for platform, a
 * type is chosen which may depend on implementation platform architecture.
 * Type chosen based on alignment size restrictions from <sys/isa_defs.h>.
 * We desire to force up to (but no more than) 64-bit (8 byte) alignment,
 * on platforms where it is possible to do so. (e.g not possible on ia32).
 * For all currently supported platforms by our implementation
 * in <sys/isa_defs.h>, (i.e. sparc, sparcv9, ia32, ia64)
 * type "double" is suitable for that intent.
 *
 * Note: Type "double" is chosen over the more obvious integer type int64_t.
 *   int64_t is not a valid type for strict ANSI/ISO C compilation on ILP32.
 */
typedef double          sockaddr_maxalign_t;

#define _SS_ALIGNSIZE   (sizeof (sockaddr_maxalign_t))

/*
 * Definitions used for sockaddr_storage structure paddings design.
 */
#define _SS_PAD1SIZE    (_SS_ALIGNSIZE - sizeof (sa_family_t))
#define _SS_PAD2SIZE    (_SS_MAXSIZE - (sizeof (sa_family_t)+ \
                        _SS_PAD1SIZE + _SS_ALIGNSIZE))

struct sockaddr_storage {
  sa_family_t     ss_family;      /* Address family */
  /* Following fields are implementation specific */
  char            _ss_pad1[_SS_PAD1SIZE];
  sockaddr_maxalign_t _ss_align;
  char            _ss_pad2[_SS_PAD2SIZE];
};

/* For SIOC[GS]LIFLNKINFO */
typedef struct lif_ifinfo_req {
  uint8_t         lir_maxhops;
  uint32_t        lir_reachtime;          /* Reachable time in msec */
  uint32_t        lir_reachretrans;       /* Retransmission timer msec */
  uint32_t        lir_maxmtu;
} lif_ifinfo_req_t;

/* Interface name size limit */
#define LIFNAMSIZ	32

/*
 * For SIOCLIF*ND ioctls.
 *
 * The lnr_state_* fields use the ND_* neighbor reachability states.
 * The 3 different fields are for use with SIOCLIFSETND to cover the cases
 * when
 *      A new entry is created
 *      The entry already exists and the link-layer address is the same
 *      The entry already exists and the link-layer address differs
 *
 * Use ND_UNCHANGED and ND_ISROUTER_UNCHANGED to not change any state.
 */
#define ND_MAX_HDW_LEN  64
typedef struct lif_nd_req {
  struct sockaddr_storage lnr_addr;
  uint8_t                 lnr_state_create;       /* When creating */
  uint8_t                 lnr_state_same_lla;     /* Update same addr */
  uint8_t                 lnr_state_diff_lla;     /* Update w/ diff. */
  int                     lnr_hdw_len;
  int                     lnr_flags;              /* See below */
  /* padding because ia32 "long long"s are only 4-byte aligned. */
  int                     lnr_pad0;
  char                    lnr_hdw_addr[ND_MAX_HDW_LEN];
} lif_nd_req_t;

/* Interface name size limit */
#define LIFNAMSIZ       32

/*
 * Interface request structure used for socket
 * ioctl's.  All interface ioctl's must have parameter
 * definitions which begin with ifr_name.  The
 * remainder may be interface specific.
 * Note: This data structure uses 64bit type uint64_t which is not
 *       a valid type for strict ANSI/ISO C compilation for ILP32.
 *       Applications with ioctls using this structure that insist on
 *       building with strict ANSI/ISO C (-Xc) will need to be LP64.
 */
#if defined(_LP64) || (__STDC__ - 0 == 0 && !defined(_NO_LONGLONG))
struct  lifreq {
  char    lifr_name[LIFNAMSIZ];           /* if name, e.g. "en0" */
  union {
    int     lifru_addrlen;          /* for subnet/token etc */
    uint_t  lifru_ppa;              /* SIOCSLIFNAME */
  } lifr_lifru1;
#define lifr_addrlen    lifr_lifru1.lifru_addrlen
#define lifr_ppa        lifr_lifru1.lifru_ppa   /* Driver's ppa */
  uint_t  lifr_movetoindex;               /* FAILOVER/FAILBACK ifindex */
  union {
    struct  sockaddr_storage lifru_addr;
    struct  sockaddr_storage lifru_dstaddr;
    struct  sockaddr_storage lifru_broadaddr;
    struct  sockaddr_storage lifru_token;   /* With lifr_addrlen */
    struct  sockaddr_storage lifru_subnet;  /* With lifr_addrlen */
    int     lifru_index;            /* interface index */
    uint64_t lifru_flags;           /* Flags for SIOC?LIFFLAGS */
    int     lifru_metric;
    uint_t  lifru_mtu;
    char    lifru_data[1];          /* interface dependent data */
    char    lifru_enaddr[6];
    int     lif_muxid[2];           /* mux id's for arp and ip */
    struct lif_nd_req       lifru_nd_req;
    struct lif_ifinfo_req   lifru_ifinfo_req;
    char    lifru_groupname[LIFNAMSIZ]; /* SIOC[GS]LIFGROUPNAME */
    uint_t  lifru_delay;               /* SIOC[GS]LIFNOTIFYDELAY */
  } lifr_lifru;

#define lifr_addr       lifr_lifru.lifru_addr   /* address */
#define lifr_dstaddr    lifr_lifru.lifru_dstaddr /* other end of p-to-p link */
#define lifr_broadaddr  lifr_lifru.lifru_broadaddr /* broadcast address */
#define lifr_token      lifr_lifru.lifru_token  /* address token */
#define lifr_subnet     lifr_lifru.lifru_subnet /* subnet prefix */
#define lifr_index      lifr_lifru.lifru_index  /* interface index */
#define lifr_flags      lifr_lifru.lifru_flags  /* flags */
#define lifr_metric     lifr_lifru.lifru_metric /* metric */
#define lifr_mtu        lifr_lifru.lifru_mtu    /* mtu */
#define lifr_data       lifr_lifru.lifru_data   /* for use by interface */
#define lifr_enaddr     lifr_lifru.lifru_enaddr /* ethernet address */
#define lifr_index      lifr_lifru.lifru_index  /* interface index */
#define lifr_ip_muxid   lifr_lifru.lif_muxid[0]
#define lifr_arp_muxid  lifr_lifru.lif_muxid[1]
#define lifr_nd         lifr_lifru.lifru_nd_req /* SIOCLIF*ND */
#define lifr_ifinfo     lifr_lifru.lifru_ifinfo_req /* SIOC[GS]LIFLNKINFO */
#define lifr_groupname  lifr_lifru.lifru_groupname
#define lifr_delay      lifr_lifru.lifru_delay
};
#endif /* defined(_LP64) || (__STDC__ - 0 == 0 && !defined(_NO_LONGLONG)) */


/* Used by SIOCGLIFNUM. Uses same flags as in struct lifconf */
struct lifnum {
  sa_family_t     lifn_family;
  int             lifn_flags;     /* request specific interfaces */
  int             lifn_count;     /* Result */
};

/*
 * Structure used in SIOCGLIFCONF request.
 * Used to retrieve interface configuration
 * for machine (useful for programs which
 * must know all networks accessible) for a given address family.
 * Using AF_UNSPEC will retrieve all address families.
 */
struct  lifconf {
  sa_family_t     lifc_family;
  int             lifc_flags;     /* request specific interfaces */
  int             lifc_len;       /* size of associated buffer */
  union {
    caddr_t lifcu_buf;
    struct  lifreq *lifcu_req;
  } lifc_lifcu;
#define lifc_buf lifc_lifcu.lifcu_buf   /* buffer address */
#define lifc_req lifc_lifcu.lifcu_req   /* array of structures returned */
};

#ifndef _IOWRN
#define _IOWRN(x, y, t)                                                 \
            ((int)((uint32_t)(IOC_INOUT|(((t)&IOCPARM_MASK)<<16)| \
            (x<<8)|y)))
#endif

#define SIOCGLIFNUM     _IOWR('i', 130, struct lifnum)  /* get number of ifs */
#define SIOCGLIFCONF    _IOWRN('i', 120, 16)            /* get if list */
#define SIOCGLIFFLAGS   _IOWR('i', 117, struct lifreq)  /* get if flags */
#define SIOCGLIFINDEX   _IOWR('i', 133, struct lifreq)  /* get if index */

struct	ip6_hdr {
	union {
		struct ip6_hdrctl {
			uint32_t	ip6_un1_flow;   /* 4 bits version, */
							/* 8 bits tclass, and */
							/* 20 bits flow-ID */
			uint16_t	ip6_un1_plen;   /* payload length */
			uint8_t		ip6_un1_nxt;    /* next header */
			uint8_t		ip6_un1_hlim;   /* hop limit */
		} ip6_un1;
		uint8_t	ip6_un2_vfc;	/* 4 bits version and */
					/* top 4 bits of tclass */
	} ip6_ctlun;
	struct in6_addr ip6_src;	/* source address */
	struct in6_addr ip6_dst;	/* destination address */
};
typedef struct ip6_hdr	ip6_t;

#define	ip6_vfc		ip6_ctlun.ip6_un2_vfc	/* 4 bits version and */
						/* top 4 bits of tclass */
#define	ip6_flow	ip6_ctlun.ip6_un1.ip6_un1_flow
#define	ip6_vcf		ip6_flow		/* Version, tclass, flow-ID */
#define	ip6_plen	ip6_ctlun.ip6_un1.ip6_un1_plen
#define	ip6_nxt		ip6_ctlun.ip6_un1.ip6_un1_nxt
#define	ip6_hlim	ip6_ctlun.ip6_un1.ip6_un1_hlim
#define	ip6_hops	ip6_ctlun.ip6_un1.ip6_un1_hlim

/* Hop-by-Hop options header */
struct ip6_hbh {
	uint8_t	ip6h_nxt;	/* next header */
	uint8_t	ip6h_len;	/* length in units of 8 octets */
		/* followed by options */
};
typedef struct ip6_hbh	ip6_hbh_t;

/* Destination options header */
struct ip6_dest {
	uint8_t	ip6d_nxt;	/* next header */
	uint8_t	ip6d_len;	/* length in units of 8 octets */
		/* followed by options */
};
typedef struct ip6_dest	ip6_dest_t;

/* Routing header */
struct ip6_rthdr {
	uint8_t	ip6r_nxt;	/* next header */
	uint8_t	ip6r_len;	/* length in units of 8 octets */
	uint8_t	ip6r_type;	/* routing type */
	uint8_t	ip6r_segleft;	/* segments left */
		/* followed by routing type specific data */
};
typedef struct ip6_rthdr	ip6_rthdr_t;

/* Type 0 Routing header */
struct ip6_rthdr0 {
	uint8_t	ip6r0_nxt;		/* next header */
	uint8_t	ip6r0_len;		/* length in units of 8 octets */
	uint8_t	ip6r0_type;		/* always zero */
	uint8_t	ip6r0_segleft;		/* segments left */
	uint8_t	ip6r0_reserved;		/* reserved field */
	uint8_t	ip6r0_slmap[3];		/* strict/loose bit map */
	struct	in6_addr ip6r0_addr[1];	/* up to 23 addresses */
};
typedef struct ip6_rthdr0	ip6_rthdr0_t;

/* Fragment header */
struct ip6_frag {
	uint8_t		ip6f_nxt;	/* next header */
	uint8_t		ip6f_reserved;	/* reserved field */
	uint16_t	ip6f_offlg;	/* offset, reserved, and flag */
	uint32_t	ip6f_ident;	/* identification */
};
typedef struct ip6_frag	ip6_frag_t;

/* ip6f_offlg field related constants (in network byte order) */
#ifdef _BIG_ENDIAN
#define	IP6F_OFF_MASK		0xfff8	/* mask out offset from _offlg */
#define	IP6F_RESERVED_MASK	0x0006	/* reserved bits in ip6f_offlg */
#define	IP6F_MORE_FRAG		0x0001	/* more-fragments flag */
#else
#define	IP6F_OFF_MASK		0xf8ff	/* mask out offset from _offlg */
#define	IP6F_RESERVED_MASK	0x0600	/* reserved bits in ip6f_offlg */
#define	IP6F_MORE_FRAG		0x0100	/* more-fragments flag */
#endif

/* IPv6 options */
struct	ip6_opt {
	uint8_t	ip6o_type;
	uint8_t	ip6o_len;
};

/*
 * The high-order 3 bits of the option type define the behavior
 * when processing an unknown option and whether or not the option
 * content changes in flight.
 */
#define	IP6OPT_TYPE(o)		((o) & 0xc0)
#define	IP6OPT_TYPE_SKIP	0x00
#define	IP6OPT_TYPE_DISCARD	0x40
#define	IP6OPT_TYPE_FORCEICMP	0x80
#define	IP6OPT_TYPE_ICMP	0xc0
#define	IP6OPT_MUTABLE		0x20

#define	IP6OPT_PAD1			0x00	/* 00 0 00000 */
#define	IP6OPT_PADN			0x01	/* 00 0 00001 */
#define	IP6OPT_JUMBO			0xc2	/* 11 0 00010 = 194 */
#define	IP6OPT_NSAP_ADDR		0xc3	/* 11 0 00011 */
#define	IP6OPT_TUNNEL_LIMIT		0x04	/* 00 0 00100 */
#define	IP6OPT_ROUTER_ALERT		0x05	/* 00 0 00101 */
#define	IP6OPT_BINDING_UPDATE		0xc6	/* 11 0 00110 */
#define	IP6OPT_BINDING_ACK		0x07	/* 00 0 00111 */
#define	IP6OPT_BINDING_REQ		0x08	/* 00 0 01000 */
#define	IP6OPT_HOME_ADDRESS		0xc9	/* 11 0 01001 */
#define	IP6OPT_EID			0x8a	/* 10 0 01010 */

/* Jumbo Payload Option */
struct	ip6_opt_jumbo {
	uint8_t	ip6oj_type;
	uint8_t	ip6oj_len;
	uint8_t ip6oj_jumbo_len[4];
};
#define	IP6OPT_JUMBO_LEN	6

/* NSAP Address Option */
struct	ip6_opt_nsap {
	uint8_t	ip6on_type;
	uint8_t	ip6on_len;
	uint8_t ip6on_src_nsap_len;
	uint8_t ip6on_dst_nsap_len;
	/* Followed by source NSAP */
	/* Followed by destination NSAP */
};

/* Tunnel Limit Option */
struct	ip6_opt_tunnel {
	uint8_t	ip6ot_type;
	uint8_t	ip6ot_len;
	uint8_t ip6ot_encap_limit;
};

/* Router Alert Option */
struct	ip6_opt_router {
	uint8_t	ip6or_type;
	uint8_t	ip6or_len;
	uint8_t ip6or_value[2];
};

/* Router alert values (in network byte order) */
#ifdef _BIG_ENDIAN
#define	IP6_ALERT_MLD			0x0000
#define	IP6_ALERT_RSVP			0x0001
#define	IP6_ALERT_AN			0x0002
#else
#define	IP6_ALERT_MLD			0x0000
#define	IP6_ALERT_RSVP			0x0100
#define	IP6_ALERT_AN			0x0200
#endif

/* Binding Update Option */
struct	ip6_opt_binding_update {
	uint8_t	ip6ou_type;
	uint8_t	ip6ou_len;
	uint8_t ip6ou_flags;
	uint8_t ip6ou_prefixlen;
	uint8_t ip6ou_seqno[2];
	uint8_t ip6ou_lifetime[4];
	uint8_t ip6ou_coa[16];		/* Optional based on flags */
	/* Followed by sub-options */
};

/* Binding Update Flags */
#define	IP6_BUF_ACK	0x80	/* Request a binding ack */
#define	IP6_BUF_HOME	0x40	/* Home Registration */
#define	IP6_BUF_COA	0x20	/* Care-of-address present in option */
#define	IP6_BUF_ROUTER	0x10	/* Sending mobile node is a router */

/* Binding Ack Option */
struct	ip6_opt_binding_ack {
	uint8_t	ip6oa_type;
	uint8_t	ip6oa_len;
	uint8_t ip6oa_status;
	uint8_t ip6oa_seqno[2];
	uint8_t ip6oa_lifetime[4];
	uint8_t ip6oa_refresh[4];
	/* Followed by sub-options */
};

/* Binding Request Option */
struct	ip6_opt_binding_request {
	uint8_t	ip6or_type;
	uint8_t	ip6or_len;
	/* Followed by sub-options */
};

/* Home Address Option */
struct	ip6_opt_home_address {
	uint8_t	ip6oh_type;
	uint8_t	ip6oh_len;
	uint8_t ip6oh_addr[16];		/* Home Address */
	/* Followed by sub-options */
};

/*
 * Type and code definitions for ICMPv6.
 * Based on RFC2292.
 */

#define	IPPROTO_ICMPV6		58		/* ICMP for IPv6 */

#define	ICMP6_INFOMSG_MASK		0x80 /* all informational messages */

/* Minimum ICMPv6 header length. */
#define	ICMP6_MINLEN	8

typedef struct icmp6_hdr {
	uint8_t	 icmp6_type;	/* type field */
	uint8_t	 icmp6_code;	/* code field */
	uint16_t icmp6_cksum;	/* checksum field */
	union {
		uint32_t icmp6_un_data32[1];	/* type-specific field */
		uint16_t icmp6_un_data16[2];	/* type-specific field */
		uint8_t	 icmp6_un_data8[4];	/* type-specific field */
	} icmp6_dataun;
} icmp6_t;

#define	icmp6_data32	icmp6_dataun.icmp6_un_data32
#define	icmp6_data16	icmp6_dataun.icmp6_un_data16
#define	icmp6_data8	icmp6_dataun.icmp6_un_data8
#define	icmp6_pptr	icmp6_data32[0]	/* parameter prob */
#define	icmp6_mtu	icmp6_data32[0]	/* packet too big */
#define	icmp6_id	icmp6_data16[0]	/* echo request/reply */
#define	icmp6_seq	icmp6_data16[1]	/* echo request/reply */
#define	icmp6_maxdelay	icmp6_data16[0]	/* mcast group membership */

/* For the Multicast Listener Discovery messages */
typedef struct icmp6_mld {
	icmp6_t		icmp6m_hdr;
	struct in6_addr	icmp6m_group; /* group address */
} icmp6_mld_t;

#define	mld_type	icmp6m_hdr.icmp6_type
#define	mld_code	icmp6m_hdr.icmp6_code
#define	mld_cksum	icmp6m_hdr.icmp6_cksum
#define	mld_maxdelay	icmp6m_hdr.icmp6_data16[0]
#define	mld_reserved	icmp6m_hdr.icmp6_data16[1]
#define	mld_group_addr	icmp6m_group

/* ICMPv6 error types */
#define	ICMP6_DST_UNREACH		1
#define	ICMP6_PACKET_TOO_BIG		2
#define	ICMP6_TIME_EXCEEDED		3
#define	ICMP6_PARAM_PROB		4

/* ICMPv6 query types */
#define	ICMP6_ECHO_REQUEST		128
#define	ICMP6_ECHO_REPLY		129

/* ICMPv6 group membership types */
#define	ICMP6_MEMBERSHIP_QUERY		130
#define	ICMP6_MEMBERSHIP_REPORT		131
#define	ICMP6_MEMBERSHIP_REDUCTION	132

/* types for neighbor discovery */
#define	ND_ROUTER_SOLICIT		133
#define	ND_ROUTER_ADVERT		134
#define	ND_NEIGHBOR_SOLICIT		135
#define	ND_NEIGHBOR_ADVERT		136
#define	ND_REDIRECT			137
#define	ICMP6_MAX_INFO_TYPE		137

#define	ICMP6_IS_ERROR(x) ((x) < 128)

/* codes for ICMP6_DST_UNREACH */
#define	ICMP6_DST_UNREACH_NOROUTE	0 /* no route to destination */
#define	ICMP6_DST_UNREACH_ADMIN		1 /* communication with destination */
					/* administratively prohibited */
#define	ICMP6_DST_UNREACH_NOTNEIGHBOR	2 /* not a neighbor */
#define	ICMP6_DST_UNREACH_ADDR		3 /* address unreachable */
#define	ICMP6_DST_UNREACH_NOPORT	4 /* bad port */

/* codes for ICMP6_TIME_EXCEEDED */
#define	ICMP6_TIME_EXCEED_TRANSIT	0 /* Hop Limit == 0 in transit */
#define	ICMP6_TIME_EXCEED_REASSEMBLY	1 /* Reassembly time out */

/* codes for ICMP6_PARAM_PROB */
#define	ICMP6_PARAMPROB_HEADER		0 /* erroneous header field */
#define	ICMP6_PARAMPROB_NEXTHEADER	1 /* unrecognized Next Header */
#define	ICMP6_PARAMPROB_OPTION		2 /* unrecognized IPv6 option */

/* Default MLD max report delay value */
#define	ICMP6_MAX_HOST_REPORT_DELAY	10	/* max delay for response to */
						/* query (in seconds)   */

typedef struct nd_router_solicit {	/* router solicitation */
	icmp6_t		nd_rs_hdr;
	/* could be followed by options */
} nd_router_solicit_t;

#define	nd_rs_type	nd_rs_hdr.icmp6_type
#define	nd_rs_code	nd_rs_hdr.icmp6_code
#define	nd_rs_cksum	nd_rs_hdr.icmp6_cksum
#define	nd_rs_reserved	nd_rs_hdr.icmp6_data32[0]

typedef struct nd_router_advert {	/* router advertisement */
	icmp6_t		nd_ra_hdr;
	uint32_t	nd_ra_reachable;   /* reachable time */
	uint32_t	nd_ra_retransmit;  /* retransmit timer */
	/* could be followed by options */
} nd_router_advert_t;

#define	nd_ra_type		nd_ra_hdr.icmp6_type
#define	nd_ra_code		nd_ra_hdr.icmp6_code
#define	nd_ra_cksum		nd_ra_hdr.icmp6_cksum
#define	nd_ra_curhoplimit	nd_ra_hdr.icmp6_data8[0]
#define	nd_ra_flags_reserved	nd_ra_hdr.icmp6_data8[1]

#define	ND_RA_FLAG_OTHER	0x40
#define	ND_RA_FLAG_MANAGED	0x80

#define	nd_ra_router_lifetime    nd_ra_hdr.icmp6_data16[1]

typedef struct nd_neighbor_solicit {   /* neighbor solicitation */
	icmp6_t		nd_ns_hdr;
	struct in6_addr nd_ns_target; /* target address */
	/* could be followed by options */
} nd_neighbor_solicit_t;

#define	nd_ns_type		nd_ns_hdr.icmp6_type
#define	nd_ns_code		nd_ns_hdr.icmp6_code
#define	nd_ns_cksum		nd_ns_hdr.icmp6_cksum
#define	nd_ns_reserved		nd_ns_hdr.icmp6_data32[0]

typedef struct nd_neighbor_advert {	/* neighbor advertisement */
	icmp6_t		  nd_na_hdr;
	struct in6_addr   nd_na_target; /* target address */
	/* could be followed by options */
} nd_neighbor_advert_t;

#define	nd_na_type	nd_na_hdr.icmp6_type
#define	nd_na_code	nd_na_hdr.icmp6_code
#define	nd_na_cksum	nd_na_hdr.icmp6_cksum

#define	nd_na_flags_reserved	nd_na_hdr.icmp6_data32[0]

/*
 * The first three bits of the flgs_reserved field of the ND structure are
 * defined in this order:
 *	Router flag
 *	Solicited flag
 * 	Override flag
 */

/* Save valuable htonl() cycles on little-endian boxen. */

#ifdef _BIG_ENDIAN

#define	ND_NA_FLAG_ROUTER	0x80000000
#define	ND_NA_FLAG_SOLICITED	0x40000000
#define	ND_NA_FLAG_OVERRIDE	0x20000000

#else /* _BIG_ENDIAN */

#define	ND_NA_FLAG_ROUTER	0x80
#define	ND_NA_FLAG_SOLICITED	0x40
#define	ND_NA_FLAG_OVERRIDE	0x20

#endif /* _BIG_ENDIAN */

typedef struct nd_redirect {	/* redirect */
	icmp6_t		nd_rd_hdr;
	struct in6_addr	nd_rd_target; /* target address */
	struct in6_addr	nd_rd_dst;    /* destination address */
	/* could be followed by options */
} nd_redirect_t;

#define	nd_rd_type	nd_rd_hdr.icmp6_type
#define	nd_rd_code	nd_rd_hdr.icmp6_code
#define	nd_rd_cksum	nd_rd_hdr.icmp6_cksum
#define	nd_rd_reserved	nd_rd_hdr.icmp6_data32[0]

typedef struct nd_opt_hdr {	/* Neighbor discovery option header */
	uint8_t	nd_opt_type;
	uint8_t	nd_opt_len;	/* in units of 8 octets */
	/* followed by option specific data */
} nd_opt_hdr_t;

/* Neighbor discovery option types */
#define	ND_OPT_SOURCE_LINKADDR		1
#define	ND_OPT_TARGET_LINKADDR		2
#define	ND_OPT_PREFIX_INFORMATION	3
#define	ND_OPT_REDIRECTED_HEADER	4
#define	ND_OPT_MTU			5

typedef struct nd_opt_prefix_info {	/* prefix information */
	uint8_t   nd_opt_pi_type;
	uint8_t   nd_opt_pi_len;
	uint8_t   nd_opt_pi_prefix_len;
	uint8_t   nd_opt_pi_flags_reserved;
	uint32_t  nd_opt_pi_valid_time;
	uint32_t  nd_opt_pi_preferred_time;
	uint32_t  nd_opt_pi_reserved2;
	struct in6_addr  nd_opt_pi_prefix;
} nd_opt_prefix_info_t;

#define	ND_OPT_PI_FLAG_AUTO	0x40
#define	ND_OPT_PI_FLAG_ONLINK	0x80

typedef struct nd_opt_rd_hdr {	/* redirected header */
	uint8_t   nd_opt_rh_type;
	uint8_t   nd_opt_rh_len;
	uint16_t  nd_opt_rh_reserved1;
	uint32_t  nd_opt_rh_reserved2;
	/* followed by IP header and data */
} nd_opt_rd_hdr_t;

typedef struct nd_opt_mtu {	/* MTU option */
	uint8_t   nd_opt_mtu_type;
	uint8_t   nd_opt_mtu_len;
	uint16_t  nd_opt_mtu_reserved;
	uint32_t  nd_opt_mtu_mtu;
} nd_opt_mtu_t;

/* Note: the option is variable length (at least 8 bytes long) */
#ifndef ND_MAX_HDW_LEN
#define	ND_MAX_HDW_LEN	64
#endif
struct nd_opt_lla {
	uint8_t	nd_opt_lla_type;
	uint8_t	nd_opt_lla_len;	/* in units of 8 octets */
	uint8_t	nd_opt_lla_hdw_addr[ND_MAX_HDW_LEN];
};


/* Neighbor discovery protocol constants */

/* Router constants */
#define	ND_MAX_INITIAL_RTR_ADVERT_INTERVAL	16000	/* milliseconds */
#define	ND_MAX_INITIAL_RTR_ADVERTISEMENTS	3	/* transmissions */
#define	ND_MAX_FINAL_RTR_ADVERTISEMENTS		3	/* transmissions */
#define	ND_MIN_DELAY_BETWEEN_RAS		3000	/* milliseconds */
#define	ND_MAX_RA_DELAY_TIME			500	/* milliseconds */

/* Host constants */
#define	ND_MAX_RTR_SOLICITATION_DELAY		1000	/* milliseconds */
#define	ND_RTR_SOLICITATION_INTERVAL		4000	/* milliseconds */
#define	ND_MAX_RTR_SOLICITATIONS		3	/* transmissions */

/* Node constants */
#define	ND_MAX_MULTICAST_SOLICIT		3	/* transmissions */
#define	ND_MAX_UNICAST_SOLICIT			3	/* transmissions */
#define	ND_MAX_ANYCAST_DELAY_TIME		1000	/* milliseconds */
#define	ND_MAX_NEIGHBOR_ADVERTISEMENT		3	/* transmissions */
#define	ND_REACHABLE_TIME			30000	/* milliseconds */
#define	ND_RETRANS_TIMER			1000	/* milliseconds */
#define	ND_DELAY_FIRST_PROBE_TIME		5000	/* milliseconds */
#define	ND_MIN_RANDOM_FACTOR			.5
#define	ND_MAX_RANDOM_FACTOR			1.5

#define	ND_MAX_REACHTIME			3600000	/* milliseconds */
#define	ND_MAX_REACHRETRANSTIME			100000	/* milliseconds */

/*
 * ICMPv6 type filtering for IPPROTO_ICMPV6 ICMP6_FILTER socket option
 */
#define	ICMP6_FILTER	0x01	/* Set filter */

typedef struct icmp6_filter {
	uint32_t	__icmp6_filt[8];
} icmp6_filter_t;

/* Pass all ICMPv6 messages to the application */
#define	ICMP6_FILTER_SETPASSALL(filterp) ( \
	((filterp)->__icmp6_filt[0] = 0xFFFFFFFFU), \
	((filterp)->__icmp6_filt[1] = 0xFFFFFFFFU), \
	((filterp)->__icmp6_filt[2] = 0xFFFFFFFFU), \
	((filterp)->__icmp6_filt[3] = 0xFFFFFFFFU), \
	((filterp)->__icmp6_filt[4] = 0xFFFFFFFFU), \
	((filterp)->__icmp6_filt[5] = 0xFFFFFFFFU), \
	((filterp)->__icmp6_filt[6] = 0xFFFFFFFFU), \
	((filterp)->__icmp6_filt[7] = 0xFFFFFFFFU))

/* ICMPv6 messages are blocked from being passed to the application */
#define	ICMP6_FILTER_SETBLOCKALL(filterp) ( \
	((filterp)->__icmp6_filt[0] = 0x0), \
	((filterp)->__icmp6_filt[1] = 0x0), \
	((filterp)->__icmp6_filt[2] = 0x0), \
	((filterp)->__icmp6_filt[3] = 0x0), \
	((filterp)->__icmp6_filt[4] = 0x0), \
	((filterp)->__icmp6_filt[5] = 0x0), \
	((filterp)->__icmp6_filt[6] = 0x0), \
	((filterp)->__icmp6_filt[7] = 0x0))

/* Pass messages of a given type to the application */
#define	ICMP6_FILTER_SETPASS(type, filterp) \
	((((filterp)->__icmp6_filt[(type) >> 5]) |= (1 << ((type) & 31))))

/* Block messages of a given type from being passed to the application */
#define	ICMP6_FILTER_SETBLOCK(type, filterp) \
	((((filterp)->__icmp6_filt[(type) >> 5]) &= ~(1 << ((type) & 31))))

/* Test if message of a given type will be passed to an application */
#define	ICMP6_FILTER_WILLPASS(type, filterp) \
	((((filterp)->__icmp6_filt[(type) >> 5]) & (1 << ((type) & 31))) != 0)

/*
 * Test if message of a given type will blocked from
 * being passed to an application
 */
#define	ICMP6_FILTER_WILLBLOCK(type, filterp) \
	((((filterp)->__icmp6_filt[(type) >> 5]) & (1 << ((type) & 31))) == 0)
#endif
