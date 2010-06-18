#include "system.h"
#include "/GadLib/FrexxLayout.h"

#define WIDTH 12

struct TagItem Tags1[] =
{
	FL_GadgetKind, BUTTON_KIND,
	FL_GadgetText, (ULONG)"1",
	FL_LeftEdge, 8,
	FL_TopEdge, 2,
	FL_Columns, WIDTH,
	FL_Rows, 1,
	TAG_END
};

struct TagItem Tags2[] =
{
	FL_GadgetKind, BUTTON_KIND,
	FL_GadgetText, (ULONG)"2",
	FL_RelLeftEdge, 1,
	FL_TopEdge, 2,
	FL_Columns, WIDTH,
	FL_Rows, 1,
	TAG_END
};

struct TagItem Tags3[] =
{
	FL_GadgetKind, BUTTON_KIND,
	FL_GadgetText, (ULONG)"3",
	FL_RelLeftEdge, 2,
	FL_TopEdge, 2,
	FL_Columns, WIDTH,
	FL_Rows, 1,
	TAG_END
};

struct TagItem Tags4[] =
{
	FL_GadgetKind, BUTTON_KIND,
	FL_GadgetText, (ULONG)"4",
	FL_LeftEdge, 8,
	FL_RelTopEdge, 1,
	FL_Columns, WIDTH,
	FL_Rows, 1,
	TAG_END
};

struct TagItem Tags5[] =
{
	FL_GadgetKind, BUTTON_KIND,
	FL_GadgetText, (ULONG)"5",
	FL_RelLeftEdge, 1,
	FL_RelTopEdge, 1,
	FL_Columns, WIDTH,
	FL_Rows, 1,
	TAG_END
};

struct TagItem Tags6[] =
{
	FL_GadgetKind, BUTTON_KIND,
	FL_GadgetText, (ULONG)"6",
	FL_RelLeftEdge, 2,
	FL_RelTopEdge, 1,
	FL_Columns, WIDTH,
	FL_Rows, 1,
	TAG_END
};

struct TagItem Tags7[] =
{
	FL_GadgetKind, BUTTON_KIND,
	FL_GadgetText, (ULONG)"7",
	FL_LeftEdge, 8,
	FL_RelTopEdge, 4,
	FL_Columns, WIDTH,
	FL_Rows, 1,
	TAG_END
};

struct TagItem Tags8[] =
{
	FL_GadgetKind, BUTTON_KIND,
	FL_GadgetText, (ULONG)"8",
	FL_RelLeftEdge, 1,
	FL_RelTopEdge, 4,
	FL_Columns, WIDTH,
	FL_Rows, 1,
	TAG_END
};

struct TagItem Tags9[] =
{
	FL_GadgetKind, BUTTON_KIND,
	FL_GadgetText, (ULONG)"9",
	FL_RelLeftEdge, 2,
	FL_RelTopEdge, 4,
	FL_Columns, WIDTH,
	FL_Rows, 1,
	TAG_END
};

struct LayoutGadgets Gads[] =
{
	{ 1, Tags1, NULL, NULL },
	{ 2, Tags2, NULL, NULL },
	{ 3, Tags3, NULL, NULL },
	{ 4, Tags4, NULL, NULL },
	{ 5, Tags5, NULL, NULL },
	{ 6, Tags6, NULL, NULL },
	{ 7, Tags7, NULL, NULL },
	{ 8, Tags8, NULL, NULL },
	{ 9, Tags9, NULL, NULL },
	{ -1, NULL, NULL, NULL }
};
