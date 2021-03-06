/*
 * This file is part of ccid-utils
 * Copyright (c) 2008 Gianni Tedesco <gianni@scaramanga.co.uk>
 * Released under the terms of the GNU GPL version 3
*/

#include <ccid.h>

#include <stdio.h>

#if 0
static int jcop_select(cci_t cci)
{
	xfr_t xfr;
	int ret;

	xfr = xfr_alloc(1024, 1024);
	if ( NULL == xfr )
		return 0;

	xfr_reset(xfr);
	xfr_tx_byte(xfr, 0); /* cla */
	xfr_tx_byte(xfr, 0xc0); /* ins: SELECT */
	xfr_tx_byte(xfr, 4); /* p1 */
	xfr_tx_byte(xfr, 0); /* p2 */
	xfr_tx_byte(xfr, 9); /* lc */
	xfr_tx_buf(xfr, (uint8_t *)"\xa0\0\0\x01\x67\x41\x30\0\xff", 9);
	ret = cci_transact(cci, xfr);

	xfr_free(xfr);
	return ret;
}
#endif

static int do_stuff(cci_t cci)
{
	const uint8_t *ats;
	size_t ats_len;

#if 0
	if ( !cci_wait_for_card(cci) ) {
		printf(" - wait failed\n");
		return 0;
	}
#endif
	ats = cci_power_on(cci, CHIPCARD_AUTO_VOLTAGE, &ats_len);
	if ( NULL == ats ) {
		printf(" - power on failed\n");
		return 0;
	}

	printf(" - power on OK\n");

#if 0
	if ( !jcop_select(cci) ) {
		printf(" - jcop select failed\n");
		return 0;
	}
#endif

	cci_power_off(cci);
	return 1;
}

static int found_ccid(ccidev_t dev)
{
	static unsigned int count;
	unsigned int i, num_slots;
	char fn[128];
	ccid_t ccid;
	cci_t cci;
	int ret = 0;

	printf("Found CCI device at %d.%d\n",
		libccid_device_bus(dev),
		libccid_device_addr(dev));

	snprintf(fn, sizeof(fn), "cselect.%u.trace", count);
	ccid = ccid_probe(dev, fn);
	if ( NULL == ccid )
		goto out;

	count++;
	printf("%s\n", ccid_name(ccid));

	num_slots = ccid_num_slots(ccid);
	printf("CCID has %d slots\n", num_slots);
	for(i = 0; i < num_slots; i++) {
		printf(" Probing slot %d\n", i);
		cci = ccid_get_slot(ccid, i);

		if ( !do_stuff(cci) )
			continue;
	}

	num_slots = ccid_num_fields(ccid);
	printf("CCID has %d RF fields\n", num_slots);
	for(i = 0; i < num_slots; i++) {
		printf(" Probing field %d\n", i);
		cci = ccid_get_field(ccid, i);

		if ( !do_stuff(cci) )
			continue;
	}

	ret = 1;
	ccid_close(ccid);
out:
	return ret;
}

int main(int argc, char **argv)
{
	ccidev_t *dev;
	size_t num_dev, i;

	dev = libccid_get_device_list(&num_dev);
	if ( NULL == dev )
		return EXIT_FAILURE;

	for(i = 0; i < num_dev; i++) {
		found_ccid(dev[i]);
		printf("\n");
	}

	libccid_free_device_list(dev);

	return EXIT_SUCCESS;
}
