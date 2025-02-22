/* Pango
 * testmisc.c: Test program for miscellaneous things
 *
 * Copyright (C) 2020 Matthias Clasen
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include <glib.h>
#include <pango/pangocairo.h>

/* test that we don't crash in shape_tab when the layout
 * is such that we don't have effective attributes
 */
static void
test_shape_tab_crash (void)
{
  PangoContext *context;
  PangoLayout *layout;

  context = pango_font_map_create_context (pango_cairo_font_map_get_default ());
  layout = pango_layout_new (context);
  pango_layout_set_text (layout, "one\ttwo", -1);
  pango_layout_is_ellipsized (layout);

  g_object_unref (layout);
  g_object_unref (context);
}

/* Test that itemizing a string with 0 characters works
 */
static void
test_itemize_empty_crash (void)
{
  PangoContext *context;

  context = pango_font_map_create_context (pango_cairo_font_map_get_default ());
  pango_itemize_with_base_dir (context, PANGO_DIRECTION_LTR, "", 0, 1, NULL, NULL);

  g_object_unref (context);
}

static void
test_itemize_utf8 (void)
{
  PangoContext *context;
  GList *result = NULL;

  context = pango_font_map_create_context (pango_cairo_font_map_get_default ());
  result = pango_itemize_with_base_dir (context, PANGO_DIRECTION_LTR, "\xc3\xa1\na", 3, 1, NULL, NULL);
  g_assert (result != NULL);

  g_list_free_full (result, (GDestroyNotify)pango_item_free);
  g_object_unref (context);
}

/* Test that pango_layout_set_text (layout, "short", 200)
 * does not lead to a crash. (pidgin does this)
 */
static void
test_short_string_crash (void)
{
  PangoContext *context;
  PangoLayout *layout;
  int width, height;

  context = pango_font_map_create_context (pango_cairo_font_map_get_default ());
  layout = pango_layout_new (context);
  pango_layout_set_text (layout, "short text", 200);
  pango_layout_get_pixel_size (layout, &width, &height);

  g_object_unref (layout);
  g_object_unref (context);
}

static void
test_language_emoji_crash (void)
{
  PangoLanguage *lang;
  const PangoScript *scripts;
  int num;

  lang = pango_language_from_string ("und-zsye");
  scripts = pango_language_get_scripts (lang, &num);

  g_assert (num >= 0);
  g_assert (scripts == NULL || num > 0);
}

static void
test_line_height (void)
{
  PangoContext *context;
  PangoLayout *layout;
  PangoLayoutLine *line;
  int height = 0;

  context = pango_font_map_create_context (pango_cairo_font_map_get_default ());
  layout = pango_layout_new (context);
  pango_layout_set_text (layout, "one\ttwo", -1);
  line = pango_layout_get_line_readonly (layout, 0);
  pango_layout_line_get_height (line, &height);

  g_assert_cmpint (height, >, 0);

  g_object_unref (layout);
  g_object_unref (context);
}

static void
test_attr_list_update (void)
{
  PangoAttribute *weight_attr;
  PangoAttribute *fg_attr;
  PangoAttrList *list;

  weight_attr = pango_attr_weight_new (700);
  weight_attr->start_index = 4;
  weight_attr->end_index = 6;

  fg_attr = pango_attr_foreground_new (0, 0, 65535);
  fg_attr->start_index = PANGO_ATTR_INDEX_FROM_TEXT_BEGINNING;
  fg_attr->end_index = PANGO_ATTR_INDEX_TO_TEXT_END;

  list = pango_attr_list_new();
  pango_attr_list_insert (list, weight_attr);
  pango_attr_list_insert (list, fg_attr);

  g_assert_cmpuint (weight_attr->start_index, ==, 4);
  g_assert_cmpuint (weight_attr->end_index, ==, 6);
  g_assert_cmpuint (fg_attr->start_index, ==, PANGO_ATTR_INDEX_FROM_TEXT_BEGINNING);
  g_assert_cmpuint (fg_attr->end_index, ==, PANGO_ATTR_INDEX_TO_TEXT_END);

  // Delete 1 byte at position 2
  pango_attr_list_update (list, 2, 1, 0);

  g_assert_cmpuint (weight_attr->start_index, ==, 3);
  g_assert_cmpuint (weight_attr->end_index, ==, 5);
  g_assert_cmpuint (fg_attr->start_index, ==, PANGO_ATTR_INDEX_FROM_TEXT_BEGINNING);
  g_assert_cmpuint (fg_attr->end_index, ==, PANGO_ATTR_INDEX_TO_TEXT_END);

  pango_attr_list_unref (list);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/layout/shape-tab-crash", test_shape_tab_crash);
  g_test_add_func ("/layout/itemize-empty-crash", test_itemize_empty_crash);
  g_test_add_func ("/layout/itemize-utf8", test_itemize_utf8);
  g_test_add_func ("/layout/short-string-crash", test_short_string_crash);
  g_test_add_func ("/language/emoji-crash", test_language_emoji_crash);
  g_test_add_func ("/layout/line-height", test_line_height);
  g_test_add_func ("/attr-list/update", test_attr_list_update);

  return g_test_run ();
}
