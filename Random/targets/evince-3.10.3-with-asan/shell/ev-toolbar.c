/* ev-toolbar.h
 *  this file is part of evince, a gnome document viewer
 *
 * Copyright (C) 2012 Carlos Garcia Campos <carlosgc@gnome.org>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ev-toolbar.h"

#include <glib/gi18n.h>
#include "ev-stock-icons.h"
#include "ev-zoom-action.h"
#include "ev-history-action.h"
#include "ev-application.h"
#include "ev-recent-menu-model.h"
#include <math.h>

enum
{
        PROP_0,
        PROP_WINDOW
};

struct _EvToolbarPrivate {
        EvWindow  *window;

        GtkWidget *view_menu_button;
        GtkWidget *action_menu_button;
        GMenu *bookmarks_section;
};

G_DEFINE_TYPE (EvToolbar, ev_toolbar, GTK_TYPE_TOOLBAR)

static void
ev_toolbar_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
        EvToolbar *ev_toolbar = EV_TOOLBAR (object);

        switch (prop_id) {
        case PROP_WINDOW:
                ev_toolbar->priv->window = g_value_get_object (value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        }
}

static void
ev_toolbar_set_button_action (EvToolbar   *ev_toolbar,
                              GtkButton   *button,
                              const gchar *action_name,
                              const gchar *tooltip)
{
        gtk_actionable_set_action_name (GTK_ACTIONABLE (button), action_name);
        gtk_button_set_label (button, NULL);
        gtk_button_set_focus_on_click (button, FALSE);
        gtk_widget_set_tooltip_text (GTK_WIDGET (button), tooltip);
}

static GtkWidget *
ev_toolbar_create_button (EvToolbar   *ev_toolbar,
                          const gchar *action_name,
                          const gchar *icon_name,
                          const gchar *tooltip)
{
        GtkWidget *button = gtk_button_new ();
        GtkWidget *image;

        image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_MENU);

        gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
        gtk_button_set_image (GTK_BUTTON (button), image);
        ev_toolbar_set_button_action (ev_toolbar, GTK_BUTTON (button), action_name, tooltip);

        return button;
}

static GtkWidget *
ev_toolbar_create_toggle_button (EvToolbar *ev_toolbar,
                                 const gchar *action_name,
                                 const gchar *icon_name,
                                 const gchar *tooltip)
{
        GtkWidget *button = gtk_toggle_button_new ();
        GtkWidget *image;

        image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_MENU);

        gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
        gtk_button_set_image (GTK_BUTTON (button), image);
        ev_toolbar_set_button_action (ev_toolbar, GTK_BUTTON (button), action_name, tooltip);

        return button;
}

static GtkWidget *
ev_toolbar_create_menu_button (EvToolbar   *ev_toolbar,
                               const gchar *icon_name,
                               GMenuModel  *menu,
                               GtkAlign     menu_align)
{
        GtkWidget *button;
        GtkMenu *popup;

        button = gtk_menu_button_new ();
        gtk_widget_set_valign (button, GTK_ALIGN_CENTER);
        gtk_button_set_image (GTK_BUTTON (button), gtk_image_new ());
        gtk_image_set_from_icon_name (GTK_IMAGE (gtk_button_get_image (GTK_BUTTON (button))),
                                      icon_name, GTK_ICON_SIZE_MENU);
        gtk_menu_button_set_menu_model (GTK_MENU_BUTTON (button), menu);

        popup = gtk_menu_button_get_popup (GTK_MENU_BUTTON (button));
        gtk_widget_set_halign (GTK_WIDGET (popup), menu_align);

        return button;
}

static GtkWidget *
ev_toolbar_create_button_group (EvToolbar *ev_toolbar)
{
        GtkStyleContext *style_context;
        GtkWidget *box;

        box = gtk_box_new (gtk_orientable_get_orientation (GTK_ORIENTABLE (ev_toolbar)), 0);

        style_context = gtk_widget_get_style_context (box);
        gtk_style_context_add_class (style_context, GTK_STYLE_CLASS_RAISED);
        gtk_style_context_add_class (style_context, GTK_STYLE_CLASS_LINKED);

        return box;
}

static void
ev_toolbar_update_bookmarks (EvToolbar *toolbar)
{
        GMenu *bookmarks_section;
        GMenuModel *bookmarks_submenu;

        /* The bookmarks section has one or two items: "Add Bookmark"
         * and the "Bookmarks" submenu item. Hide the latter when there
         * are no bookmarks.
         */

        bookmarks_section = toolbar->priv->bookmarks_section;
        bookmarks_submenu = ev_window_get_bookmarks_menu (toolbar->priv->window);

        if (g_menu_model_get_n_items (bookmarks_submenu) > 0) {
                if (g_menu_model_get_n_items (G_MENU_MODEL (bookmarks_section)) == 1)
                        g_menu_append_submenu (bookmarks_section, _("Bookmarks"), bookmarks_submenu);
        }
        else {
                if (g_menu_model_get_n_items (G_MENU_MODEL (bookmarks_section)) == 2)
                        g_menu_remove (bookmarks_section, 1);
        }
}

static void
ev_toolbar_constructed (GObject *object)
{
        EvToolbar      *ev_toolbar = EV_TOOLBAR (object);
        GtkActionGroup *action_group;
        GtkWidget      *tool_item;
        GtkWidget      *hbox;
        GtkAction      *action;
        GtkWidget      *button;
        GMenuModel     *menu;

        G_OBJECT_CLASS (ev_toolbar_parent_class)->constructed (object);

        /* Set the MENUBAR style class so it's possible to drag the app
         * using the toolbar. */
        gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (ev_toolbar)),
                                     GTK_STYLE_CLASS_MENUBAR);

        action_group = ev_window_get_main_action_group (ev_toolbar->priv->window);

        /* Navigation */
        hbox = ev_toolbar_create_button_group (ev_toolbar);

        button = ev_toolbar_create_button (ev_toolbar, "win.go-previous-page",
                                           "go-up-symbolic", _("Go to the previous page"));
        gtk_container_add (GTK_CONTAINER (hbox), button);
        gtk_widget_show (button);

        button = ev_toolbar_create_button (ev_toolbar, "win.go-next-page",
                                           "go-down-symbolic", _("Go to the next page"));
        gtk_container_add (GTK_CONTAINER (hbox), button);
        gtk_widget_show (button);

        tool_item = GTK_WIDGET (gtk_tool_item_new ());
        gtk_widget_set_margin_right (tool_item, 12);
        gtk_container_add (GTK_CONTAINER (tool_item), hbox);
        gtk_widget_show (hbox);

        gtk_container_add (GTK_CONTAINER (ev_toolbar), tool_item);
        gtk_widget_show (tool_item);

        /* Page selector */
        action = gtk_action_group_get_action (action_group, "PageSelector");
        tool_item = gtk_action_create_tool_item (action);
        gtk_widget_set_margin_right (tool_item, 12);
        gtk_container_add (GTK_CONTAINER (ev_toolbar), tool_item);
        gtk_widget_show (tool_item);

        /* History */
        action = gtk_action_group_get_action (action_group, "History");
        tool_item = gtk_action_create_tool_item (action);
        gtk_widget_set_margin_right (tool_item, 12);
        gtk_container_add (GTK_CONTAINER (ev_toolbar), tool_item);
        gtk_widget_show (tool_item);

        /* Separator */
        tool_item = GTK_WIDGET (gtk_tool_item_new ());
        gtk_tool_item_set_expand (GTK_TOOL_ITEM (tool_item), TRUE);
        gtk_container_add (GTK_CONTAINER (ev_toolbar), tool_item);
        gtk_widget_show (tool_item);

        /* Find */
        button = ev_toolbar_create_toggle_button (ev_toolbar, "win.find", "edit-find-symbolic",
                                                  _("Find a word or phrase in the document"));
        tool_item = GTK_WIDGET (gtk_tool_item_new ());
        gtk_container_add (GTK_CONTAINER (tool_item), button);
        gtk_widget_show (button);
        gtk_widget_set_margin_right (tool_item, 12);
        gtk_container_add (GTK_CONTAINER (ev_toolbar), tool_item);
        gtk_widget_show (tool_item);

        /* Zoom selector */
        action = gtk_action_group_get_action (action_group, "ViewZoom");
        tool_item = gtk_action_create_tool_item (action);
        gtk_widget_set_margin_right (tool_item, 12);
        gtk_container_add (GTK_CONTAINER (ev_toolbar), tool_item);
        gtk_widget_show (tool_item);

        if (!ev_application_has_traditional_menus (EV_APP)) {
                GtkBuilder *builder;
                GMenu *recent_submenu;
                GMenuModel *recent_menu_model;

                builder = gtk_builder_new_from_resource ("/org/gnome/evince/shell/ui/menus.ui");

                /* View Menu */
                menu = G_MENU_MODEL (gtk_builder_get_object (builder, "view-menu"));
                button = ev_toolbar_create_menu_button (ev_toolbar, "document-properties-symbolic",
                                                        menu, GTK_ALIGN_END);
                ev_toolbar->priv->view_menu_button = button;
                tool_item = GTK_WIDGET (gtk_tool_item_new ());
                gtk_widget_set_margin_left (tool_item, 12);
                gtk_container_add (GTK_CONTAINER (tool_item), button);
                gtk_widget_show (button);
                gtk_widget_set_margin_right (tool_item, 6);

                gtk_container_add (GTK_CONTAINER (ev_toolbar), tool_item);
                gtk_widget_show (tool_item);

                /* Action Menu */
                menu = G_MENU_MODEL (gtk_builder_get_object (builder, "action-menu"));
                button = ev_toolbar_create_menu_button (ev_toolbar, "emblem-system-symbolic",
                                                        menu, GTK_ALIGN_END);
                ev_toolbar->priv->action_menu_button = button;
                tool_item = GTK_WIDGET (gtk_tool_item_new ());
                gtk_container_add (GTK_CONTAINER (tool_item), button);
                gtk_widget_show (button);

                gtk_container_add (GTK_CONTAINER (ev_toolbar), tool_item);
                gtk_widget_show (tool_item);

                /* insert dynamic recent files submenu */
                recent_menu_model = ev_recent_menu_model_new (gtk_recent_manager_get_default (),
                                                              "app.open-file",
                                                              g_get_application_name ());
                recent_submenu = G_MENU (gtk_builder_get_object (builder, "recent"));
                g_menu_append_section (recent_submenu, NULL, recent_menu_model);

                /* insert bookmarks section */
                ev_toolbar->priv->bookmarks_section = G_MENU (gtk_builder_get_object (builder, "bookmarks"));
                g_signal_connect_swapped (ev_window_get_bookmarks_menu (ev_toolbar->priv->window), "items-changed",
                                          G_CALLBACK (ev_toolbar_update_bookmarks), ev_toolbar);
                ev_toolbar_update_bookmarks (ev_toolbar);

                g_object_unref (recent_menu_model);
                g_object_unref (builder);
        }
}

static void
ev_toolbar_class_init (EvToolbarClass *klass)
{
        GObjectClass *g_object_class = G_OBJECT_CLASS (klass);

        g_object_class->set_property = ev_toolbar_set_property;
        g_object_class->constructed = ev_toolbar_constructed;

        g_object_class_install_property (g_object_class,
                                         PROP_WINDOW,
                                         g_param_spec_object ("window",
                                                              "Window",
                                                              "The evince window",
                                                              EV_TYPE_WINDOW,
                                                              G_PARAM_WRITABLE |
                                                              G_PARAM_CONSTRUCT_ONLY |
                                                              G_PARAM_STATIC_STRINGS));

        g_type_class_add_private (g_object_class, sizeof (EvToolbarPrivate));
}

static void
ev_toolbar_init (EvToolbar *ev_toolbar)
{
        ev_toolbar->priv = G_TYPE_INSTANCE_GET_PRIVATE (ev_toolbar, EV_TYPE_TOOLBAR, EvToolbarPrivate);
}

GtkWidget *
ev_toolbar_new (EvWindow *window)
{
        g_return_val_if_fail (EV_IS_WINDOW (window), NULL);

        return GTK_WIDGET (g_object_new (EV_TYPE_TOOLBAR,
                                         "window", window,
                                         NULL));
}

gboolean
ev_toolbar_has_visible_popups (EvToolbar *ev_toolbar)
{
        GtkAction        *action;
        GtkActionGroup   *action_group;
        GtkMenu          *popup_menu;
        EvToolbarPrivate *priv;

        g_return_val_if_fail (EV_IS_TOOLBAR (ev_toolbar), FALSE);

        priv = ev_toolbar->priv;

        popup_menu = gtk_menu_button_get_popup (GTK_MENU_BUTTON (priv->view_menu_button));
        if (gtk_widget_get_visible (GTK_WIDGET (popup_menu)))
                return TRUE;

        popup_menu = gtk_menu_button_get_popup (GTK_MENU_BUTTON (priv->action_menu_button));
        if (gtk_widget_get_visible (GTK_WIDGET (popup_menu)))
                return TRUE;

        action_group = ev_window_get_main_action_group (ev_toolbar->priv->window);
        action = gtk_action_group_get_action (action_group, "ViewZoom");
        if (ev_zoom_action_get_popup_shown (EV_ZOOM_ACTION (action)))
                return TRUE;

        action = gtk_action_group_get_action (action_group, "History");
        if (ev_history_action_get_popup_shown (EV_HISTORY_ACTION (action)))
                return TRUE;

        return FALSE;
}

void
ev_toolbar_action_menu_popup (EvToolbar *ev_toolbar)
{
        g_return_if_fail (EV_IS_TOOLBAR (ev_toolbar));

        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ev_toolbar->priv->action_menu_button),
                                      TRUE);
}
