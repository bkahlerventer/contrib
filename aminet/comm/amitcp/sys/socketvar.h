/*
 * $Id$
 *
 * Copyright (c) 1993 AmiTCP/IP Group, <amitcp-group@hut.fi>
 *                    Helsinki University of Technology, Finland.
 *                    All rights reserved.
 *
 * HISTORY
 * $Log$
 * Revision 1.1  2001/12/25 22:28:13  henrik
 * amitcp
 *
 * Revision 1.15  1993/06/04  11:16:15  jraja
 * Fixes for first public release.
 *
 * Revision 1.14  1993/05/30  08:30:42  too
 * Changed struct selitem to struct newselitem
 *
 * Revision 1.13  1993/05/17  01:02:04  ppessi
 * Changed RCS version
 *
 * Revision 1.12  1993/04/24  22:52:52  jraja
 * Removed #ifdef USECLUSTERS (now using clusters always)
 *
 * Revision 1.11  93/04/12  09:22:35  09:22:35  jraja (Jarno Tapio Rajahalme)
 * Changed sp_pgid to struct SocketBase *.
 * 
 * Revision 1.10  93/04/11  22:22:38  22:22:38  jraja (Jarno Tapio Rajahalme)
 * Added parenthesis around && inside || for clarity.
 * 
 * Revision 1.9  93/03/03  16:12:41  16:12:41  jraja (Jarno Tapio Rajahalme)
 * Cleanup. Changed u_long sb_max to extern. Removed sonewconn macro.
 * 
 * Revision 1.8  93/02/26  16:30:08  16:30:08  jraja (Jarno Tapio Rajahalme)
 * fixed include and changed proc to selitem.
 * 
 * Revision 1.7  93/02/25  15:03:38  15:03:38  too (Tomi Ollila)
 *  2 spaces removed 
 * 
 * Revision 1.6  93/02/24  16:26:27  16:26:27  too (Tomi Ollila)
 * Changed so_linger and sb_timeout types from short to struct timeval
 * 
 * Revision 1.5  93/02/24  16:12:58  16:12:58  too (Tomi Ollila)
 * returned SockatBase pointer.. one can use NULL here.
 * 
 * Revision 1.4  93/02/24  16:04:43  16:04:43  too (Tomi Ollila)
 * removed SocketBase pointer.. too many problems occurred.
 * 
 * Revision 1.3  93/02/23  11:35:44  11:35:44  too (Tomi Ollila)
 * added SocketBase pointer to sblock so sb_lock can use AMITCP tsleep code
 * 
 * Revision 1.2  93/02/04  18:17:15  18:17:15  jraja (Jarno Tapio Rajahalme)
 * Added #ifdef USECLUSTERS to remove mbuf cluster dependant code.
 * 
 * Revision 1.1  92/11/20  15:42:39  15:42:39  jraja (Jarno Tapio Rajahalme)
 * Initial revision
 * 
 *
 */

/*-
 * Copyright (c) 1982, 1986, 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)socketvar.h	7.17 (Berkeley) 5/5/91
 */

#ifndef SYS_SOCKETVAR_H
#define SYS_SOCKETVAR_H

#ifndef SYS_TIME_H
#include <sys/time.h>
#endif

struct newselitem;

/*
 * Kernel structure per socket.
 * Contains send and receive buffer queues,
 * handle on protocol and pointer to protocol
 * private data and error information.
 */
struct socket {
	short	so_type;		/* generic type, see socket.h */
	short	so_options;		/* from socket call, see socket.h */
	struct timeval so_linger;	/* time to linger while closing */
	short	so_state;		/* internal state flags SS_*, below */
	short   so_refcnt;              /* reference count for this socket */
	caddr_t	so_pcb;			/* protocol control block */
	struct	protosw *so_proto;	/* protocol handle */
/*
 * Variables for connection queueing.
 * Socket where accepts occur is so_head in all subsidiary sockets.
 * If so_head is 0, socket is not related to an accept.
 * For head socket so_q0 queues partially completed connections,
 * while so_q is a queue of connections ready to be accepted.
 * If a connection is aborted and it has so_head set, then
 * it has to be pulled out of either so_q0 or so_q.
 * We allow connections to queue up based on current queue lengths
 * and limit on number of queued connections for this socket.
 */
	struct	socket *so_head;	/* back pointer to accept socket */
	struct	socket *so_q0;		/* queue of partial connections */
	struct	socket *so_q;		/* queue of incoming connections */
	short	so_q0len;		/* partials on so_q0 */
	short	so_qlen;		/* number of connections on so_q */
	short	so_qlimit;		/* max number queued connections */
	short	so_timeo;		/* connection timeout */
	u_short	so_error;		/* error affecting connection */
	struct  SocketBase *so_pgid;	/* owner for signals */
	u_long	so_oobmark;		/* chars to oob mark */
/*
 * Variables for socket buffering.
 */
	struct	sockbuf {
		u_long	sb_cc;		     /* actual chars in buffer */
		u_long	sb_hiwat;	     /* max actual char count */
		u_long	sb_mbcnt;	     /* chars of mbufs used */
		u_long	sb_mbmax;	     /* max chars of mbufs to use */
		long	sb_lowat;	     /* low water mark */
		struct	mbuf *sb_mb;	     /* the mbuf chain */
		struct	newselitem *sb_sel;  /* chain for selecting tasks */
		short	sb_flags;	     /* flags, see below */
		struct timeval sb_timeo;     /* timeout for read/write */
	} so_rcv, so_snd;
#define	SB_MAX		(64*1024)	/* default for max chars in sockbuf */
#define	SB_LOCK		0x01		/* lock on data queue */
#define	SB_WANT		0x02		/* someone is waiting to lock */
#define	SB_WAIT		0x04		/* someone is waiting for data/space */
#define	SB_SEL		0x08		/* someone is selecting */
#define	SB_ASYNC	0x10		/* ASYNC I/O, need signals */
#define	SB_NOTIFY	(SB_WAIT|SB_SEL|SB_ASYNC)
#define	SB_COLL		0x20		/* collision selecting */
#define	SB_NOINTR	0x40		/* operations not interruptible */

	caddr_t	so_tpcb;		/* Wisc. protocol control block XXX */
};

/*
 * Socket state bits.
 */
#define	SS_NOFDREF		0x001	/* no file table ref any more */
#define	SS_ISCONNECTED		0x002	/* socket connected to a peer */
#define	SS_ISCONNECTING		0x004	/* in process of connecting to peer */
#define	SS_ISDISCONNECTING	0x008	/* in process of disconnecting */
#define	SS_CANTSENDMORE		0x010	/* can't send more data to peer */
#define	SS_CANTRCVMORE		0x020	/* can't receive more data from peer */
#define	SS_RCVATMARK		0x040	/* at mark on input */

#define	SS_PRIV			0x080	/* privileged for broadcast, raw... */
#define	SS_NBIO			0x100	/* non-blocking ops */
#define	SS_ASYNC		0x200	/* async i/o notify */
#define	SS_ISCONFIRMING		0x400	/* deciding to accept connection req */


/*
 * Macros for sockets and socket buffering.
 */

/*
 * How much space is there in a socket buffer (so->so_snd or so->so_rcv)?
 * This is problematical if the fields are unsigned, as the space might
 * still be negative (cc > hiwat or mbcnt > mbmax).  Should detect
 * overflow and return 0.  Should use "lmin" but it doesn't exist now.
 */
#define	sbspace(sb) \
    ((long) imin((int)((sb)->sb_hiwat - (sb)->sb_cc), \
	 (int)((sb)->sb_mbmax - (sb)->sb_mbcnt)))

/* do we have to send all at once on a socket? */
#define	sosendallatonce(so) \
    ((so)->so_proto->pr_flags & PR_ATOMIC)

/* can we read something from so? */
#define	soreadable(so) \
    ((so)->so_rcv.sb_cc >= (so)->so_rcv.sb_lowat || \
	((so)->so_state & SS_CANTRCVMORE) || \
	(so)->so_qlen || (so)->so_error)

/* can we write something to so? */
#define	sowriteable(so) \
    ((sbspace(&(so)->so_snd) >= (so)->so_snd.sb_lowat && \
      (((so)->so_state&SS_ISCONNECTED) || \
       ((so)->so_proto->pr_flags&PR_CONNREQUIRED)==0)) \
     || ((so)->so_state & SS_CANTSENDMORE) \
     || (so)->so_error)

/* adjust counters in sb reflecting allocation of m */
#define	sballoc(sb, m) { \
	(sb)->sb_cc += (m)->m_len; \
	(sb)->sb_mbcnt += MSIZE; \
	if ((m)->m_flags & M_EXT) \
		(sb)->sb_mbcnt += (m)->m_ext.ext_size; \
}

/* adjust counters in sb reflecting freeing of m */
#define	sbfree(sb, m) { \
	(sb)->sb_cc -= (m)->m_len; \
	(sb)->sb_mbcnt -= MSIZE; \
	if ((m)->m_flags & M_EXT) \
		(sb)->sb_mbcnt -= (m)->m_ext.ext_size; \
}

/*
 * Set lock on sockbuf sb; sleep if lock is already held.
 * Unless SB_NOINTR is set on sockbuf, sleep is interruptible.
 * Returns error without lock if sleep is interrupted.
 */
#define sblock(sb, p) ((sb)->sb_flags & SB_LOCK ? sb_lock(sb, p) : \
		((sb)->sb_flags |= SB_LOCK, 0))

/* release lock on sockbuf sb */
#define	sbunlock(sb) { \
	(sb)->sb_flags &= ~SB_LOCK; \
	if ((sb)->sb_flags & SB_WANT) { \
		(sb)->sb_flags &= ~SB_WANT; \
		wakeup((caddr_t)&(sb)->sb_flags); \
	} \
}

#define	sorwakeup(so)	sowakeup((so), &(so)->so_rcv)
#define	sowwakeup(so)	sowakeup((so), &(so)->so_snd)

#ifdef KERNEL
extern u_long	sb_max;

/* strings for sleep message: */
extern	char netio[], netcon[], netcls[];

#endif
#endif /* !SYS_SOCKETVAR_H */





