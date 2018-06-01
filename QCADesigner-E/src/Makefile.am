## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = QCADesigner batch_sim graph_dialog

UNDO_REDO_FILES = \
	objects/QCADUndoEntry.c      objects/QCADUndoEntry.h \
	objects/QCADUndoEntryGroup.c objects/QCADUndoEntryGroup.h \
	selection_undo.c             selection_undo.h

STDIO_FILEIO_FILES = \
	fileio.c fileio.h

OBJECT_FILES = \
	objects/QCADCell.c           objects/QCADCell.h \
	objects/QCADCompoundDO.c     objects/QCADCompoundDO.h \
	objects/QCADDesignObject.c   objects/QCADDesignObject.h \
	objects/QCADDOContainer.c    objects/QCADDOContainer.h \
	objects/QCADLabel.c          objects/QCADLabel.h \
	objects/QCADLayer.c          objects/QCADLayer.h \
	objects/QCADRuler.c          objects/QCADRuler.h \
	objects/QCADStretchyObject.c objects/QCADStretchyObject.h \
	objects/QCADSubstrate.c      objects/QCADSubstrate.h

#################### QCADesigner ####################
QCADesigner_CFLAGS = \
	-DSTDIO_FILEIO \
	-DUNDO_REDO \
	-DGTK_GUI \
	-DDESIGNER \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@GTK_CFLAGS@ \
	@GTHREAD_CFLAGS@

QCADesigner_SOURCES = \
	$(UNDO_REDO_FILES) \
	$(STDIO_FILEIO_FILES) \
	gtk_preamble.c                          gtk_preamble.h \
	about.c                                 about.h \
	actions.h \
	actions/action_handlers.h \
	actions/array.c                         actions/array.h \
	actions/pan.c                           actions/pan.h \
	actions/rotate.c                        actions/rotate.h \
	actions/select.c                        actions/select.h \
	bistable_properties_dialog.c            bistable_properties_dialog.h \
	bus_layout_dialog.c                     bus_layout_dialog.h \
	                                        callback_helpers.h \
	callbacks.c                             callbacks.h \
	coherence_vector_properties_dialog.c    coherence_vector_properties_dialog.h \
	coherence_vector_energy_properties_dialog.c    coherence_vector_energy_properties_dialog.h \
	custom_widgets.c                        custom_widgets.h \
	design.c                                design.h \
	exp_array.c                             exp_array.h \
	exp_pixmap.c                            exp_pixmap.h \
	file_selection_window.c                 file_selection_window.h \
	fileio_helpers.c                        fileio_helpers.h \
	generic_utils.c                         generic_utils.h \
	                                        global_consts.h \
	honeycomb_thresholds_dialog.c           honeycomb_thresholds_dialog.h \
	graph_dialog.c                          graph_dialog.h \
	graph_dialog_interface.c                graph_dialog_interface.h \
	graph_dialog_widget_data.c              graph_dialog_widget_data.h \
	graph_dialog_data.c                     graph_dialog_data.h \
	graph_dialog_callbacks.c                graph_dialog_callbacks.h \
	hsl2rgb.c                               hsl2rgb.h \
	interface.c                             interface.h \
	layer_mapping_dialog.c                  layer_mapping_dialog.h \
	layer_order_dialog.c                    layer_order_dialog.h \
	layer_properties_dialog.c               layer_properties_dialog.h \
	objects/mouse_handlers.c                objects/mouse_handlers.h \
	                                    	  objects/mouse_handler_struct.h \
	objects/object_helpers.c                objects/object_helpers.h \
	                                    	  objects/objects_debug.h \
\
	$(OBJECT_FILES) \
\
	objects/QCADClockCombo.c                objects/QCADClockCombo.h \
	objects/QCADCellRendererVT.c            objects/QCADCellRendererVT.h \
	objects/QCADCellRendererLayerList.c     objects/QCADCellRendererLayerList.h \
	objects/QCADTreeViewContainer.c         objects/QCADTreeViewContainer.h \
	objects/QCADPrintDialog.c               objects/QCADPrintDialog.h \
	print_properties_dialog.c               print_properties_dialog.h \
	print_graph_properties_dialog.c         print_graph_properties_dialog.h \
	                                        print_properties_dialog_callbacks.h \
	print_properties_dialog_interface.c     print_properties_dialog_interface.h \
	print.c                                 print.h \
	print_util.c                            print_util.h \
	print_graph.c \
	                                        qcadstock.h \
	scale_dialog.c                          scale_dialog.h \
	selection_renderer.c                    selection_renderer.h \
	sim_engine_setup_dialog.c               sim_engine_setup_dialog.h \
	support.c                               support.h \
	translate_selection_dialog.c            translate_selection_dialog.h \
	vector_table.c                          vector_table.h \
	vector_table_options_dialog.c           vector_table_options_dialog.h \
	vector_table_options_dialog_interface.c vector_table_options_dialog_interface.h \
	vector_table_options_dialog_callbacks.c vector_table_options_dialog_callbacks.h \
	vector_table_options_dialog_data.c      vector_table_options_dialog_data.h \
	recent_files.c                          recent_files.h \
	print_preview.c                         print_preview.h \
	bistable_simulation.c                   bistable_simulation.h \
	coherence_vector.c                      coherence_vector.h \
	coherence_vector_energy.c               coherence_vector_energy.h \
	simulation.c                            simulation.h \
	simulation_data.c                       simulation_data.h \
	main.c

#
#	sim_type_setup_dialog.c                 sim_type_setup_dialog.h \
#

QCADesigner_LDADD = \
	-lm \
	@GTK_LIBS@ \
	$(INTLLIBS) \
	@GTHREAD_LIBS@ 

#################### batch_sim ######################
batch_sim_CFLAGS = \
	-DSTDIO_FILEIO \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@GLIB_CFLAGS@ \
	@GOBJECT_CFLAGS@

batch_sim_SOURCES = \
	$(STDIO_FILEIO_FILES) \
	fileio_helpers.c                      fileio_helpers.h \
	custom_widgets.c                      custom_widgets.h \
\
	simulation.c                          simulation.h \
	simulation_data.c                     simulation_data.h \
	bistable_simulation.c                 bistable_simulation.h \
	coherence_vector.c                    coherence_vector.h \
	coherence_vector_energy.c             coherence_vector_energy.h \
	design.c                              design.h \
\
	$(OBJECT_FILES) \
\
	exp_array.c                           exp_array.h \
	objects/object_helpers.c              objects/object_helpers.h \
	hsl2rgb.c                             hsl2rgb.h \
	generic_utils.c                       generic_utils.h \
\
	graph_dialog_widget_data.c            graph_dialog_widget_data.h \
\
	main_batch_sim.c

#
#	graph_dialog.c                        graph_dialog.h \
#	exp_pixmap.c                          exp_pixmap.h \
#	file_selection_window.c               file_selection_window.h \
#	print.c                               print.h \
#	print_util.c                          print_util.h \
#	print_graph.c \
#	objects/QCADPrintDialog.c             objects/QCADPrintDialog.h \
#	print_graph_properties_dialog.c       print_graph_properties_dialog.h \
#	support.c                             support.h \
#	honeycomb_thresholds_dialog.c         honeycomb_thresholds_dialog.h \
#	bus_layout_dialog.c                   bus_layout_dialog.h \
#	print_preview.c                       print_preview.h \
#	objects/QCADClockCombo.c              objects/QCADClockCombo.h \
#

batch_sim_LDADD = \
	-lm \
	@GLIB_LIBS@ \
	@GOBJECT_LIBS@

#################### graph_dialog ###################
graph_dialog_CFLAGS = \
	-DSTDIO_FILEIO \
	-DGTK_GUI \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@GTK_CFLAGS@ \
	@GTHREAD_CFLAGS@

graph_dialog_SOURCES = \
	$(STDIO_FILEIO_FILES) \
	$(OBJECT_FILES) \
	gtk_preamble.c                        gtk_preamble.h \
	objects/QCADClockCombo.c              objects/QCADClockCombo.h \
	objects/QCADPrintDialog.c             objects/QCADPrintDialog.h \
	design.c                              design.h \
	objects/object_helpers.c              objects/object_helpers.h \
\
	graph_dialog.c                        graph_dialog.h \
	graph_dialog_interface.c              graph_dialog_interface.h \
	graph_dialog_widget_data.c            graph_dialog_widget_data.h \
	graph_dialog_data.c                   graph_dialog_data.h \
	graph_dialog_callbacks.c              graph_dialog_callbacks.h \
\
	generic_utils.c                       generic_utils.h \
	custom_widgets.c                      custom_widgets.h \
	exp_array.c                           exp_array.h \
	fileio_helpers.c                      fileio_helpers.h \
	exp_pixmap.c                          exp_pixmap.h \
	file_selection_window.c               file_selection_window.h \
	print.c                               print.h \
	print_util.c                          print_util.h \
	print_graph.c \
	print_graph_properties_dialog.c       print_graph_properties_dialog.h \
	support.c                             support.h \
	honeycomb_thresholds_dialog.c         honeycomb_thresholds_dialog.h \
	bus_layout_dialog.c                   bus_layout_dialog.h \
	print_preview.c                       print_preview.h \
	simulation_data.c                     simulation_data.h \
	hsl2rgb.c                             hsl2rgb.h \
	main_graph_dialog.c

graph_dialog_LDADD = \
	-lm \
	@GTK_LIBS@ \
	$(INTLLIBS) \
	@GTHREAD_LIBS@ 
