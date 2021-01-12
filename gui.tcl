package require Tk

proc main {} {
	ttk::style theme use winnative

	wm title . "Watermarker"
	wm resizable . false false

	ttk::frame .c -padding "3 3 12 12"
	grid .c -column 0 -row 0 -sticky nwes
	grid columnconfigure . 0 -weight 1
	grid rowconfigure . 0 -weight 1

	tk::button .c.add_button -text "Add Files" -command add_files
	grid .c.add_button -column 0 -row 0 -pady 10

	tk::button .c.remove_button -text "Remove Selected" -command remove_files
	grid .c.remove_button -column 1 -row 0 -pady 10

	tk::button .c.clear_button -text "Clear List" -command ".c.listbox delete 0 end"
	grid .c.clear_button -column 2 -row 0 -pady 10

	tk::listbox .c.listbox -height 20 -width 50 -selectmode extended
	grid .c.listbox -column 0 -row 1 -columnspan 3

	ttk::scrollbar .c.vertical_scrollbar -orient vertical -command ".c.listbox yview"
	.c.listbox configure -yscrollcommand ".c.vertical_scrollbar set"
	grid .c.vertical_scrollbar -column 3 -row 1 -sticky ns

	ttk::scrollbar .c.horizontal_scrollbar -orient horizontal -command ".c.listbox xview"
	.c.listbox configure -xscrollcommand ".c.horizontal_scrollbar set"
	grid .c.horizontal_scrollbar -column 0 -row 2 -columnspan 3 -sticky we

	tk::button .c.start_button -text "Start" -command start
	grid .c.start_button -column 1 -row 3 -pady 10
}

proc add_files {} {
	foreach path [tk_getOpenFile -multiple true] {
		.c.listbox insert end $path
	}
}

proc remove_files {} {
	set elements [.c.listbox curselection]
	foreach element [lreverse $elements] {
		.c.listbox delete $element
		puts $element
	}
}

proc start {} {
	set elements [.c.listbox get 0 end]
	exec watermarker {*}$elements > watermarker.log
	if {[file size watermarker.log] > 0} {
		tk_messageBox -icon warning -message "Processing finished.\nAdditional info was recorded in watermarker.log"
	} else {
		tk_messageBox -icon info -message "Processing finished."
	}
}

main
