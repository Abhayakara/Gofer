// -*- Mode: ObjC; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108 -*-
//
//  ReportUI.m
//  Gofer
//
// Created by Ted Lemon on 5/29/18.
// Copyright (c) 2018 Edward W. Lemon III

// This file is part of GOFER.

// GOFER is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// GOFER is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with GOFER, in a file called COPYING; if not, write
// to the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
// Boston, MA  02111-1307  USA
//

#import "GoferUI.h"
#import "ReportUI.h"

@implementation ReportUI

- (id)init
{
	self = [super initWithWindowNibName:@"ReportUI" owner: self];
	if (self == nil)
		printf("ReportUI initWithWindowNibName failed.\n");
	return self;
}

- (void)awakeFromNib {
    [super awakeFromNib];
	printf("awakened from nib...\n");
}

- (void)addReportStanza: (NSString *)reportStanza
{
	[[self.reportView textStorage]
		appendAttributedString: [[NSAttributedString alloc]
									initWithString: reportStanza]];
}

- (IBAction)cancelClicked:(id)sender
{

}
- (IBAction)sendClicked:(id)sender
{
}


@end
