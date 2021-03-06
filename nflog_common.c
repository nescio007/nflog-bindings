#include <arpa/inet.h>
#include <linux/netfilter.h>
#include <libnfnetlink/libnfnetlink.h>
#include <linux/ip.h>

#include <stdio.h>
#include <stdlib.h>

#include "exception.h"

#include "nflog.h"
#include "nflog_common.h"

#include "nflog_version.h"

const char * nflog_bindings_version(void)
{
	return NFLOG_BINDINGS_VERSION;
}

int log_open(struct log *self)
{
	self->_h = nflog_open();
	self->_gh = NULL;
	return (self->_h != NULL);
}

void log_close(struct log *self)
{
	nflog_close(self->_h);
	self->_gh = NULL;
	self->_h = NULL;
	self->_cb = NULL;
}

int log_bind(struct log *self, int af_family)
{
	if (nflog_bind_pf(self->_h, af_family)) {
		throw_exception("error during nflog_bind_pf()");
		return -1;
	}
	return 0;
}

int log_unbind(struct log *self, int af_family)
{
	if (nflog_unbind_pf(self->_h, af_family)) {
		throw_exception("error during nflog_unbind_pf()");
		return -1;
	}
	return 0;
}

int log_create_queue(struct log *self, int queue_num)
{
	int ret;

	if (self->_cb == NULL) {
		throw_exception("Error: no callback set");
		return -1;
	}

	self->_gh = nflog_bind_group(self->_h, queue_num);
	if (self->_gh == NULL) {
		throw_exception("error during nflog_bind_group()");
		return -1;
	}

	ret = nflog_callback_register(self->_gh, &swig_nflog_callback, (void*)self->_cb);
	if (ret != 0) {
		throw_exception("error during nflog_callback_register()");
		return -1;
	}

	return 0;
}

int log_fast_open(struct log *self, int queue_num, int af_family)
{
	int ret;

	if (self->_cb == NULL) {
		throw_exception("Error: no callback set");
		return -1;
	}

	ret = log_open(self);
	if (!ret)
		return -1;

	log_unbind(self, af_family);
	ret = log_bind(self, af_family);
	if (ret < 0) {
		log_close(self);
		return -1;
	}

	ret = log_create_queue(self,queue_num);
	if (ret < 0) {
		log_unbind(self, af_family);
		log_close(self);
		return -1;
	}

	return 0;
}

int log_set_bufsiz(struct log *self, int bufsz)
{
	int ret;
	ret = nflog_set_nlbufsiz(self->_gh, bufsz);
	if (ret < 0) {
		throw_exception("error during nflog_set_nlbufsiz()\n");
	}
	return ret;
}

int log_set_qthresh(struct log *self, uint32_t qthresh)
{
	int ret;
	ret = nflog_set_qthresh(self->_gh, qthresh);
	if (ret < 0) {
		throw_exception("error during nflog_set_qthresh()\n");
	}
	return ret;
}

int log_set_timeout(struct log *self, uint32_t timeout)
{
	int ret;
	ret = nflog_set_timeout(self->_gh, timeout);
	if (ret < 0) {
		throw_exception("error during nflog_set_timeout()\n");
	}
	return ret;
}

int log_set_flags(struct log *self, enum CfgFlags flags)
{
	int ret;
	ret = nflog_set_flags(self->_gh, flags);
	if (ret < 0) {
		throw_exception("error during nflog_set_flags()\n");
	}
	return ret;
}

int log_set_mode(struct log *self, enum CopyMode mode, uint32_t range)
{
	int ret;
	ret = nflog_set_mode(self->_gh, mode, range);
	if (ret < 0) {
		throw_exception("error during nflog_set_mode()\n");
	}
	return ret;
}

int log_stop_loop(struct log *self)
{
	self->fd = -1;

	return 0;
}

int log_prepare(struct log *self)
{
	int rv;
	int opt = 1;

	if (nflog_set_mode(self->_gh, NFULNL_COPY_PACKET, 0xffff) < 0) {
		throw_exception("can't set packet_copy mode\n");
		exit(1);
	}

	self->fd = nflog_fd(self->_h);

	/* avoid ENOBUFS on read() operation, otherwise the while loop
	* in log_loop() is interrupted. */
	rv = setsockopt(self->fd, SOL_NETLINK, NETLINK_NO_ENOBUFS, &opt, sizeof(int));
	if (rv == -1) {
		throw_exception("can't set setsockopt(NETLINK_NO_ENOBUFS)");
		exit(1);
	}

	return 0;
}

uint32_t log_payload_get_nfmark(struct log_payload *self)
{
	return nflog_get_nfmark(self->nfad);
}

struct timeval log_payload_get_timestamp(struct log_payload *self)
{
	struct timeval tv = {0,0};
	if (nflog_get_timestamp(self->nfad, &tv) < 0) {
		throw_exception("No such attribute");
		return tv;
	}
	return tv;
}

int log_payload_get_indev(struct log_payload *self)
{
	return nflog_get_indev(self->nfad);
}

int log_payload_get_physindev(struct log_payload *self)
{
	return nflog_get_physindev(self->nfad);
}

int log_payload_get_outdev(struct log_payload *self)
{
	return nflog_get_outdev(self->nfad);
}

int log_payload_get_physoutdev(struct log_payload *self)
{
	return nflog_get_physoutdev(self->nfad);
}

uint32_t log_payload_get_uid(struct log_payload *self)
{
	u_int32_t i;
	if (nflog_get_uid(self->nfad, &i) < 0) {
		throw_exception("No such attribute");
		return (uint32_t)-1;
	}
	return i;
}

uint32_t log_payload_get_gid(struct log_payload *self)
{
	u_int32_t i;
	if (nflog_get_gid(self->nfad, &i) < 0) {
		throw_exception("No such attribute");
		return (uint32_t)-1;
	}
	return i;
}

uint32_t log_payload_get_seq(struct log_payload *self)
{
	u_int32_t i;
	if (nflog_get_seq(self->nfad, &i) < 0) {
		throw_exception("No such attribute");
		return (uint32_t)-1;
	}
	return i;
}

uint32_t log_payload_get_seq_global(struct log_payload *self)
{
	u_int32_t i;
	if (nflog_get_seq_global(self->nfad, &i) < 0) {
		throw_exception("No such attribute");
		return (uint32_t)-1;
	}
	return i;
}

const char * log_payload_get_prefix(struct log_payload *self)
{
	return nflog_get_prefix(self->nfad);
}

uint16_t log_payload_get_hwtype(struct log_payload *self)
{
	return nflog_get_hwtype(self->nfad);
}
