/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
int DisMount(struct DeviceList *volume);
struct DeviceList *Mount(char *name, struct MsgPort *port);
