/*
 * panedimage.c
 *
 * Copyright 2013-2021 AmatCoder
 *
 * This file is part of Mednaffe.
 *
 * Mednaffe is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mednaffe is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mednaffe; if not, see <http://www.gnu.org/licenses/>.
 *
 *
 */


#include "panedimage.h"


typedef struct _PanedImageClass PanedImageClass;
typedef struct _PanedImagePrivate PanedImagePrivate;

struct _PanedImageClass {
  GtkPanedClass parent_class;
};

struct _PanedImagePrivate {
  GtkImage* image_a;
  GtkImage* image_b;
  gchar* path_a;
  gchar* path_b;
  gchar* file_a;
  gchar* file_b;
  gint width;
  gint height1;
  gint height2;
  gboolean isvisible;
  gboolean isvisible_a;
  gboolean isvisible_b;
  gboolean preserve_aspect_ratio;
};


G_DEFINE_TYPE_WITH_PRIVATE (PanedImage, paned_image, GTK_TYPE_PANED);


static void
paned_image_set_pixbuf (PanedImage* self,
                        GtkImage* image,
                        const gchar* path,
                        gint height)
{
  PanedImagePrivate* priv = paned_image_get_instance_private (self);

  if ((priv->width > 0) && (height > 0))
  {
    GdkPixbuf* pix = gdk_pixbuf_new_from_file_at_scale (path, priv->width, height, priv->preserve_aspect_ratio, NULL);

    if (pix != NULL)
    {
      gtk_image_set_from_pixbuf (image, pix);
      g_object_unref (pix);
    }

  }
}


static gchar*
paned_image_set_image (PanedImage* self,
                       GtkImage* image,
                       const gchar* path,
                       const gchar* filename,
                       gint height)
{
  gchar* file = g_strconcat (path, "/", filename, ".png", NULL);

  gboolean b = g_file_test (file, G_FILE_TEST_EXISTS);

  if (b)
    paned_image_set_pixbuf (self, image, file, height);
  else
  {
    gtk_image_clear (image);
    g_free (file);
    file = NULL;
  }

  return file;
}


void
paned_image_set_images (PanedImage* self,
                        const gchar* str)
{
  g_return_if_fail (self != NULL);
  g_return_if_fail (str != NULL);

  PanedImagePrivate* priv = paned_image_get_instance_private (self);

  if (priv->isvisible)
  {
    if (priv->isvisible_a)
    {
      g_free (priv->file_a);
      priv->file_a = paned_image_set_image (self, priv->image_a, priv->path_a, str, priv->height1);
    }

    if (priv->isvisible_b)
    {
      g_free (priv->file_b);
      priv->file_b = paned_image_set_image (self, priv->image_b, priv->path_b, str, priv->height2);
    }
  }
}


void
paned_image_set_paths (PanedImage* self,
                       const gchar* str_a,
                       const gchar* str_b)
{
  g_return_if_fail (self != NULL);
  g_return_if_fail (str_a != NULL);
  g_return_if_fail (str_b != NULL);

  PanedImagePrivate* priv = paned_image_get_instance_private (self);

  g_free (priv->path_a);
  priv->path_a = g_strdup (str_a);

  g_free (priv->path_b);
  priv->path_b = g_strdup (str_b);

  priv->isvisible_a = (g_strcmp0 (str_a, "") != 0);
  priv->isvisible_b = (g_strcmp0 (str_b, "") != 0);

  gtk_widget_set_visible ((GtkWidget*) priv->image_a, priv->isvisible_a);
  gtk_widget_set_visible ((GtkWidget*) priv->image_b, priv->isvisible_b);

  priv->isvisible = (priv->isvisible_a || priv->isvisible_b);
  gtk_widget_set_visible ((GtkWidget*) self, (priv->isvisible));
}


static void
paned_image_scale_images (GtkWidget* sender,
                          GtkAllocation* allocation,
                          gpointer self)
{
  PanedImage* pi = self;
  PanedImagePrivate* priv = paned_image_get_instance_private (pi);

  if (allocation->width < 16)
    return;

  if (priv->width == allocation->width - 16)
    return;

  priv->width = allocation->width - 16;
  gint tmp_heigth = (allocation->height - gtk_paned_get_position ((GtkPaned*) pi));

  if (priv->isvisible_a && priv->isvisible_b)
    priv->height2 = tmp_heigth;
  else if (!priv->isvisible_a && priv->isvisible_b)
    priv->height2 = allocation->height;
  else if (priv->isvisible_a && !priv->isvisible_b)
    priv->height2 = 0;

  priv->height1 = (allocation->height - priv->height2);

  if (priv->isvisible)
  {
    if (priv->isvisible_a)
      if (priv->file_a != NULL)
        paned_image_set_pixbuf (pi, priv->image_a, priv->file_a, priv->height1);

    if (priv->isvisible_b)
      if (priv->file_b != NULL)
        paned_image_set_pixbuf (pi, priv->image_b, priv->file_b, priv->height2);
  }
}


static void
paned_image_finalize (GObject* obj)
{
  PanedImage* self = G_TYPE_CHECK_INSTANCE_CAST (obj, paned_image_get_type (), PanedImage);
  PanedImagePrivate* priv = paned_image_get_instance_private (self);

  g_free (priv->path_a);
  g_free (priv->path_b);
  g_free (priv->file_a);
  g_free (priv->file_b);

  G_OBJECT_CLASS (paned_image_parent_class)->finalize (obj);
}


PanedImage*
paned_image_new (GtkOrientation orientation)
{
  PanedImage* self = (PanedImage*) g_object_new (paned_image_get_type(), NULL);
  PanedImagePrivate* priv = paned_image_get_instance_private (self);

  gtk_orientable_set_orientation ((GtkOrientable*) self, orientation);

  priv->path_a = NULL;
  priv->path_b = NULL;
  priv->file_a = NULL;
  priv->file_b = NULL;
  priv->width = 0;
  priv->height1 = 0;
  priv->height2 = 0;
  priv->isvisible = FALSE;
  priv->isvisible_a = FALSE;
  priv->isvisible_b = FALSE;
  priv->preserve_aspect_ratio = TRUE; //TODO: to be implemented.

  priv->image_a = (GtkImage*) gtk_image_new();
  gtk_paned_pack1 ((GtkPaned*) self, (GtkWidget*) priv->image_a, TRUE, TRUE);

  priv->image_b = (GtkImage*) gtk_image_new();
  gtk_paned_pack2 ((GtkPaned*) self, (GtkWidget*) priv->image_b, TRUE, TRUE);

  g_signal_connect_object ((GtkWidget*) self, "size-allocate", (GCallback) paned_image_scale_images, self, 0);

  gtk_widget_show_all ((GtkWidget*) self);

  return self;
}


static void
paned_image_init (PanedImage* self)
{
}


static void
paned_image_class_init (PanedImageClass* klass)
{
  G_OBJECT_CLASS (klass)->finalize = paned_image_finalize;
}
