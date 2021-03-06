/* ev-zoom-action.h
 *  this file is part of evince, a gnome document viewer
 *
 * Copyright (C) 2012 Carlos Garcia Campos  <carlosgc@gnome.org>
 *
 * Evince is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Evince is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef EV_ZOOM_ACTION_H
#define EV_ZOOM_ACTION_H

#include <gtk/gtk.h>
#include <evince-document.h>
#include <evince-view.h>

#include "ev-window.h"

G_BEGIN_DECLS

#define EV_TYPE_ZOOM_ACTION            (ev_zoom_action_get_type ())
#define EV_ZOOM_ACTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), EV_TYPE_ZOOM_ACTION, EvZoomAction))
#define EV_IS_ZOOM_ACTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EV_TYPE_ZOOM_ACTION))
#define EV_ZOOM_ACTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), EV_TYPE_ZOOM_ACTION, EvZoomActionClass))
#define EV_IS_ZOOM_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), EV_TYPE_ZOOM_ACTION))
#define EV_ZOOM_ACTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), EV_TYPE_ZOOM_ACTION, EvZoomActionClass))

typedef struct _EvZoomAction        EvZoomAction;
typedef struct _EvZoomActionClass   EvZoomActionClass;
typedef struct _EvZoomActionPrivate EvZoomActionPrivate;

struct _EvZoomAction {
        GtkAction parent;

        EvZoomActionPrivate *priv;
};

struct _EvZoomActionClass {
        GtkActionClass parent_class;
};

static const struct {
        const gchar *name;
        float        level;
} zoom_levels[] = {
        { "50%", 0.5 },
        { "70%", 0.7071067811 },
        { "85%", 0.8408964152 },
        { "100%", 1.0 },
        { "125%", 1.1892071149 },
        { "150%", 1.4142135623 },
        { "175%", 1.6817928304 },
        { "200%", 2.0 },
        { "300%", 2.8284271247 },
        { "400%", 4.0 },
        { "800%", 8.0 },
        { "1600%", 16.0 },
        { "3200%", 32.0 },
        { "6400%", 64.0 }
};

GType    ev_zoom_action_get_type        (void);

void     ev_zoom_action_set_model       (EvZoomAction    *action,
                                         EvDocumentModel *model);
gboolean ev_zoom_action_get_popup_shown (EvZoomAction    *action);
void     ev_zoom_action_set_window      (EvZoomAction    *action,
                                         EvWindow        *window);

G_END_DECLS

#endif
