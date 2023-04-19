package require Tk

proc main {} {
	ttk::style theme use winnative

	wm title . "Watermarker"
	wm resizable . false false

	ttk::frame .c -padding "3 3 12 12"
	grid .c -column 0 -row 0 -sticky nwes
	grid columnconfigure . 0 -weight 1
	grid rowconfigure . 0 -weight 1

	# File Operations
	ttk::button .c.add_button -text "Add Files" -command add_files
	grid .c.add_button -column 0 -row 0 -pady 10

	ttk::button .c.remove_button -text "Remove Selected" -command remove_files
	grid .c.remove_button -column 1 -row 0 -pady 10

	ttk::button .c.clear_button -text "Clear List" -command ".c.listbox delete 0 end"
	grid .c.clear_button -column 2 -row 0 -pady 10

	# File List
	tk::listbox .c.listbox -height 20 -width 50 -selectmode extended
	grid .c.listbox -column 0 -row 1 -columnspan 3

	ttk::scrollbar .c.vertical_scrollbar -orient vertical -command ".c.listbox yview"
	.c.listbox configure -yscrollcommand ".c.vertical_scrollbar set"
	grid .c.vertical_scrollbar -column 3 -row 1 -sticky ns

	ttk::scrollbar .c.horizontal_scrollbar -orient horizontal -command ".c.listbox xview"
	.c.listbox configure -xscrollcommand ".c.horizontal_scrollbar set"
	grid .c.horizontal_scrollbar -column 0 -row 2 -columnspan 3 -sticky we

	# Save Directory
	ttk::label .c.savedir_label -text "Output Directory"
	grid .c.savedir_label -column 0 -row 3 -pady {10 0} -sticky w

	global savedir
	set savedir [pwd]
	ttk::entry .c.savedir_entry -textvariable savedir
	grid .c.savedir_entry -column 0 -row 4 -columnspan 2 -sticky we

	ttk::button .c.savedir_button -text "Select" -command {set savedir [tk_chooseDirectory]}
	grid .c.savedir_button -column 2 -row 4

	# Mark Directory
	ttk::label .c.markfile_label -text "Mark Image"
	grid .c.markfile_label -column 0 -row 5 -pady {10 0} -sticky w

	global markfile
	set markfile [pwd]/mark.png
	ttk::entry .c.markfile_entry -textvariable markfile
	grid .c.markfile_entry -column 0 -row 6 -columnspan 2 -sticky we

	ttk::button .c.markfile_button -text "Select" -command {set markfile [tk_getOpenFile]}
	grid .c.markfile_button -column 2 -row 6

	# Start Button
	ttk::button .c.start_button -text "Start" -command start
	grid .c.start_button -column 1 -row 7 -pady 10
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
	global markfile
	if {![file exists $markfile]} {
		return -code error "mark image does not exist"
	}

	global savedir
	if {![file exists $savedir]} {
		return -code error "output directory does not exist"
	}

	if {[.c.listbox size] < 1} {
		return -code error "no files to process"
	}

	set elements [.c.listbox get 0 end]
	exec -- watermarker --savedir $savedir --markfile $markfile {*}$elements > watermarker.log
	if {[file size watermarker.log] > 0} {
		tk_messageBox -icon warning -message "Processing finished.\nAdditional info was recorded in watermarker.log"
	} else {
		tk_messageBox -icon info -message "Processing finished."
	}
}

main
