docdir = $(datadir)/doc/@PACKAGE@-@VERSION@

MANUAL_FILES = \
	manual.src/menu.txt                               \
	manual.src/manipulating_objects.html              \
	manual.src/menu.html                              \
	manual.src/adding_objects.html                    \
	manual.src/graph_dialog.html                      \
	manual.src/moving_around.html                     \
	manual.src/style.css                              \
	manual.src/clocking.html                          \
	manual.src/installation.html                      \
	manual.src/create_import_block.html               \
	manual.src/object_properties.html                 \
	manual.src/index.html                             \
	manual.src/managing_layers.html                   \
	manual.src/vector_table.html                      \
	manual.src/exhaustive_verification.html           \
	manual.src/coherence_vector_engine.html           \
	manual.src/images/traces_dialog.png               \
	manual.src/images/clock_signal.eps                \
	manual.src/images/clock_signal.png                \
	manual.src/images/fleur.png                       \
	manual.src/images/coherence_equation_motion.png   \
	manual.src/images/actions_toolbar.png             \
	manual.src/images/cell_button.png                 \
	manual.src/images/tree-null.png                   \
	manual.src/images/increment_cell_clocks.png       \
	manual.src/images/clocks_layer.png                \
	manual.src/images/exhaustive_verification.png     \
	manual.src/images/layer_properties_dialog.png     \
	manual.src/images/print_dialog.png                \
	manual.src/images/layer_properties_button.png     \
	manual.src/images/clock_signal.ai                 \
	manual.src/images/pan_button.png                  \
	manual.src/images/electrostatic_energy.png        \
	manual.src/images/qcadlogo_gradient.png           \
	manual.src/images/temperature_ratio.png           \
	manual.src/images/coherence_options.png           \
	manual.src/images/zoom_in_button.png              \
	manual.src/images/alt_style_convention.dia        \
	manual.src/images/alt_style_convention.eps        \
	manual.src/images/alt_style_convention.png        \
	manual.src/images/gamma_components.png            \
	manual.src/images/array_button.png                \
	manual.src/images/graph_dialog_zoom_in.png        \
	manual.src/images/copy_button.png                 \
	manual.src/images/cellcellresponse.png            \
	manual.src/images/shroedinger_equation.png        \
	manual.src/images/extents_button.png              \
	manual.src/images/layers_pulldown.png             \
	manual.src/images/add_layer_button.png            \
	manual.src/images/vector_table_save_vectors.png   \
	manual.src/images/alt_style_button.png            \
	manual.src/images/nonlinear_approx_formula.png    \
	manual.src/images/polarization_definition.png     \
	manual.src/images/lambda_steady_state.png         \
	manual.src/images/layer_properties_dialog_2.png   \
	manual.src/images/bistable_options.png            \
	manual.src/images/mirror_button.png               \
	manual.src/images/layer_button.png                \
	manual.src/images/layers_toolbar.png              \
	manual.src/images/two_state_hamiltonian.png       \
	manual.src/images/nonlinear_options.png           \
	manual.src/images/bus_layout_dialog.png           \
	manual.src/images/coherence_vector_components.png \
	manual.src/images/cell_functions_dialog.png       \
	manual.src/images/bus_layout_button.png           \
	manual.src/images/drawing_layer.png               \
	manual.src/images/clock_select_pulldown.png       \
	manual.src/images/graph_dialog.png                \
	manual.src/images/cell_properties_dialog.png      \
	manual.src/images/create_block_menu_item.png      \
	manual.src/images/vector_table_select.png         \
	manual.src/images/rearrange_layers_button.png     \
	manual.src/images/cells_layer.png                 \
	manual.src/images/move_button.png                 \
	manual.src/images/rotate_button.png               \
	manual.src/images/design_area.png                 \
	manual.src/images/clock_select_dialog-1.png       \
	manual.src/images/clock_select_dialog-2.png       \
	manual.src/images/clock_select_dialog-3.png       \
	manual.src/images/function_button.png             \
	manual.src/images/translate_button.png            \
	manual.src/images/layer_mapping_dialog.png        \
	manual.src/images/rotate_selection_menu.png       \
	manual.src/images/substrate_layer.png             \
	manual.src/images/zoom_out_button.png             \
	manual.src/images/radius_of_effect.png            \
	manual.src/images/qcadlogo.gif                    \
	manual.src/images/pan-step-1.png                  \
	manual.src/images/pan-step-2.png                  \
	manual.src/images/layer_order_dialog.png          \
	manual.src/images/explicit_gamma.png              \
	manual.src/images/pan-step-3.png                  \
	manual.src/images/radius_of_effect.ai             \
	manual.src/images/remove_layer_button.png         \
	manual.src/QCADesigner.ico                        \
	manual.src/hot_keys.html                          \
	manual.src/print_dialog.html                      \
	manual.src/bistable_simulation_engine.html        \
	manual.src/images/add-vector-button.png           \
	manual.src/images/insert-before-button.png        \
	manual.src/images/save-vector-table-button.png    \
	manual.src/images/inserting_vectors.png           \
	manual.src/images/editing_vectors.png             \
	manual.src/images/delete-vector-button.png        \
	manual.src/images/close-vector-table-button.png   \
	manual.src/bus_layout.html

all:manual

manual:$(MANUAL_FILES)
	rm -rf manual ; \
	mkdir manual ; \
	cd manual.src ; \
	for Nix in `find . | grep -v CVS | grep -v '^\.$$'`; do \
	if [ -d $$Nix ]; then \
		mkdir "../manual/$$Nix" ;\
	elif [ -f $$Nix ]; then \
		if echo $$Nix | grep -q '\.html$$'; then \
			if echo $$Nix | grep -qv '^\./menu\.html$$'; then \
				echo Inserting menu into file $$Nix ; \
				if echo $(build) | grep -q solaris ; then \
					echo -e "/<!--MENU-->/\nd\n.r ../manual.src/menu.html\nw ../manual/$$Nix\nq\n" | ed - $$Nix > /dev/null ; \
				else \
					echo -e "/<!--MENU-->/d\n^r ../manual.src/menu.html\nw ../manual/$$Nix\nq\n" | ed - $$Nix > /dev/null ; \
				fi ; \
			fi ; \
		elif echo $$Nix | grep -qEv '^\./menu\.(html|txt)$$'; then \
			cp $$Nix "../manual/$$Nix" ; \
		fi ; \
	fi ; \
	done ; \
	cd ..

install-data-local:manual
	$(mkinstalldirs) "$(DESTDIR)$(docdir)/manual"
	for Nix in `find manual`; do \
		if [ -d $$Nix ]; then \
			$(mkinstalldirs) "$(DESTDIR)$(docdir)/$$Nix" ;\
		elif [ -f $$Nix ]; then \
			$(INSTALL) -m 644 $$Nix "$(DESTDIR)$(docdir)/$$Nix" ; \
		fi ; \
	done

uninstall-local:
	rm -rf "$(DESTDIR)$(docdir)/manual"

clean:
	rm -rf manual

distclean:clean
	rm -f Makefile
