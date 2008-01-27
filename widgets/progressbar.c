/*
 * progressbar.c - progress bar widget
 *
 * Copyright © 2007-2008 Julien Danjou <julien@danjou.info>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <string.h>
#include "widget.h"
#include "screen.h"
#include "common/util.h"

extern AwesomeConf globalconf;

typedef struct
{
    /** Percent 0 to 100 */
    int *percent;
    /** Width of the bars */
    int width;
    /** Left padding */
    int padding_left;
    /** Pixel between bars */
    int gap;
    /** Number of bars */
    int bars;
    /** Height 0-1, where 1 is height of statusbar */
    float height;
    /** Foreground color */
    XColor *fg;
    /** Background color */
    XColor *bg;
    /** Border color */
    XColor *bordercolor;
} Data;

static int
progressbar_draw(Widget *widget, DrawCtx *ctx, int offset,
                 int used __attribute__ ((unused)))
{
    int i, width, pwidth, margin_top, pb_height, left_offset;
    Area rectangle;

    Data *d = widget->data;

    if (!(d->bars))
        return 0;

    width = d->width - d->padding_left;

    if(!widget->user_supplied_x)
        widget->area.x = widget_calculate_offset(widget->statusbar->width,
                                                 d->width,
                                                 offset,
                                                 widget->alignment);

    if(!widget->user_supplied_y)
        widget->area.y = 0;

    margin_top = (int) (widget->statusbar->height * (1 - d->height)) / 2 + 0.5 + widget->area.y;
    pb_height = (int) ((widget->statusbar->height * d->height - (d->gap * (d->bars - 1))) / d->bars + 0.5); 
    left_offset = widget->area.x + d->padding_left;

    for(i = 0; i < d->bars; i++)
    {
        pwidth = (int) d->percent[i] ? ((width - 2) * d->percent[i]) / 100 : 0;

        rectangle.x = left_offset;
        rectangle.y = margin_top;
        rectangle.width = width;
        rectangle.height = pb_height;

        draw_rectangle(ctx, rectangle, False, d->bordercolor[i]);

        if(pwidth > 0)
        {
            rectangle.x = left_offset + 1;
            rectangle.y = margin_top + 1;
            rectangle.width = pwidth;
            rectangle.height = pb_height - 2;
            draw_rectangle(ctx, rectangle, True, d->fg[i]);
        }

        if(width - 2 - pwidth > 0) /* not filled area */
        {
            rectangle.x = left_offset + 1 + pwidth;
            rectangle.y = margin_top + 1;
            rectangle.width = width - 2 - pwidth;
            rectangle.height = pb_height - 2;
            draw_rectangle(ctx, rectangle, True, d->bg[i]);
        }

        margin_top += (pb_height + d->gap);
    }

    widget->area.width = d->width;
    widget->area.height = widget->statusbar->height;
    return widget->area.width;
}

static void
progressbar_tell(Widget *widget, char *command)
{
    Data *d = widget->data;
    int i = 0, percent;
    char * tok;

    if(!command || !d->bars)
        return;

    for (tok = strtok(command, ","); tok && i < d->bars; tok = strtok(NULL, ","), i++)
    {
        percent = atoi(tok);
        d->percent[i] = (percent < 0 ? 0 : (percent > 100 ? 100 : percent));
    }
}

Widget *
progressbar_new(Statusbar *statusbar, cfg_t *config)
{
    Widget *w;
    Data *d;
    char *color;
    int i, phys_screen = get_phys_screen(statusbar->screen);
    cfg_t *cfg;


    w = p_new(Widget, 1);
    widget_common_new(w, statusbar, config);
    w->draw = progressbar_draw;
    w->tell = progressbar_tell;
    d = w->data = p_new(Data, 1);
    d->width = cfg_getint(config, "width");

    if(!(d->bars = cfg_size(config, "bar")))
    {
        warn("progressbar widget needs at least one bar section\n");
        return w;
    }

    d->bg = p_new(XColor, d->bars);
    d->fg = p_new(XColor, d->bars);
    d->bordercolor = p_new(XColor, d->bars);
    d->percent = p_new(int, d->bars);

    for(i = 0; i < d->bars; i++)
    {
        cfg = cfg_getnsec(config, "bar", i);

        if((color = cfg_getstr(cfg, "fg")))
            d->fg[i] = draw_color_new(globalconf.display, phys_screen, color);
        else
            d->fg[i] = globalconf.screens[statusbar->screen].colors_normal[ColFG];

        if((color = cfg_getstr(cfg, "bg")))
            d->bg[i] = draw_color_new(globalconf.display, phys_screen, color);
        else
            d->bg[i] = globalconf.screens[statusbar->screen].colors_normal[ColBG];

        if((color = cfg_getstr(cfg, "bordercolor")))
            d->bordercolor[i] = draw_color_new(globalconf.display, phys_screen, color);
        else
            d->bordercolor[i] = d->fg[i];

    } 

    d->height = cfg_getfloat(config, "height");
    d->gap = cfg_getint(config, "gap");
    d->padding_left = cfg_getint(config, "padding_left");

    return w;
}
// vim: filetype=c:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=80
