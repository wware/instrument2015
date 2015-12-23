// OpenSCAD file for layout of internal components in the Tooba
// This is for Toobas build Nov 2015 and later.
// Vim hacking for OpenSCAD files: https://github.com/sirtaj/vim-openscad
// Dimensions are in millimeters.
INCH = 25.4;

OVERALL_LENGTH = 24 * INCH;
INNER_DIAMETER = 2.9375 * INCH;
OUTER_DIAMETER = 3.375 * INCH;

PANEL_HEIGHT = 2 * INCH;
PANEL_WIDTH = 2 * INCH;
PANEL_Z = 2 * INCH;
theta = asin(PANEL_WIDTH / INNER_DIAMETER);
PANEL_X = 0.5 * INNER_DIAMETER * cos(theta);

PLYWOOD_HEIGHT = 9.5 * INCH;
PLYWOOD_WIDTH = 2 * INCH;
PLYWOOD_Z = 6 * INCH;
theta2 = asin(PLYWOOD_WIDTH / INNER_DIAMETER);
PLYWOOD_X = 0.5 * INNER_DIAMETER * cos(theta2);

KEY_OFFSET = (7.75 / 9) * INCH;
KEYPAD_34_HEIGHT = 10 * KEY_OFFSET;
KEYPAD_34_Z = PANEL_Z + PANEL_HEIGHT + 2 * INCH;

SEPARATION = 0 * INCH;

module halfpipe() {
    intersection() {
        translate([0, -5 * INCH, -1])
            cube([10 * INCH, 10 * INCH, OVERALL_LENGTH + 2]);
        difference() {
            cylinder(d=OUTER_DIAMETER, h=OVERALL_LENGTH);
            translate([0, 0, -1])
                cylinder(d=INNER_DIAMETER, h=OVERALL_LENGTH + 2);
        }
    }
}

module machinescrew(L) {   /* 1/4"-20 */
    color([0, 1, 0])
        rotate([0, -90, 0]) {
            cylinder(h=L, d=0.25*INCH);
            cylinder(h=0.35*INCH, d1=0.5*INCH, d2=0);
        }
}

module keyboard() {
    R = 1.6 * INCH;
    angle_diff = 25;
    for (i = [0 : 1 : 9]) {
        translate([R * cos(-1.5 * angle_diff),
                   R * sin(-1.5 * angle_diff),
                   KEYPAD_34_Z + i * KEY_OFFSET])
            sphere(d=.5*INCH);
        translate([R * cos(.5 * angle_diff),
                   R * sin(.5 * angle_diff),
                   KEYPAD_34_Z + i * KEY_OFFSET])
            sphere(d=.5*INCH);
    }
    for (i = [0 : 1 : 1]) {
        translate([R * cos(-.5 * angle_diff),
                   R * sin(-.5 * angle_diff),
                   KEYPAD_34_Z + (i + .5) * KEY_OFFSET])
            sphere(d=.5*INCH);
        translate([R * cos(1.5 * angle_diff),
                   R * sin(1.5 * angle_diff),
                   KEYPAD_34_Z + (i + .5) * KEY_OFFSET])
            sphere(d=.5*INCH);
    }
    for (i = [0 : 1 : 2]) {
        translate([R * cos(-.5 * angle_diff),
                   R * sin(-.5 * angle_diff),
                   KEYPAD_34_Z + (i + 3.5) * KEY_OFFSET])
            sphere(d=.5*INCH);
        translate([R * cos(1.5 * angle_diff),
                   R * sin(1.5 * angle_diff),
                   KEYPAD_34_Z + (i + 3.5) * KEY_OFFSET])
            sphere(d=.5*INCH);
    }
    for (i = [0 : 1 : 1]) {
        translate([R * cos(-.5 * angle_diff),
                   R * sin(-.5 * angle_diff),
                   KEYPAD_34_Z + (i + 7.5) * KEY_OFFSET])
            sphere(d=.5*INCH);
        translate([R * cos(1.5 * angle_diff),
                   R * sin(1.5 * angle_diff),
                   KEYPAD_34_Z + (i + 7.5) * KEY_OFFSET])
            sphere(d=.5*INCH);
    }
    for (i = [0 : 1 : 5]) {
        translate([R * cos(.5 * angle_diff),
                   R * sin(.5 * angle_diff),
                   KEYPAD_34_Z + (i + 10.5) * KEY_OFFSET])
            sphere(d=.5*INCH);
    }
}

module plywood() {
    color([0.9, 0.7, 0])
        translate([0, -.5 * PLYWOOD_WIDTH, 0])
            cube([0.25*INCH, PLYWOOD_WIDTH, PLYWOOD_HEIGHT]);
    color([0.2, 0.9, 0.2])
        translate([0.5*INCH, -.5 * PLYWOOD_WIDTH, 1*INCH])
            cube([0.1*INCH, PLYWOOD_WIDTH, 6*INCH]);
    color([0, 0.3, 0.5])
        translate([0.25*INCH, 0, PLYWOOD_HEIGHT - 1.5 * INCH])
            rotate([0, 90, 0])
                cylinder(h=1*INCH, d=1.5*INCH);
    translate([-0.64*INCH, 0, 0.5*INCH])
        mirror([1, 0, 0])
            machinescrew(1.25 * INCH);
    translate([-0.65*INCH, 0, PLYWOOD_HEIGHT - 0.5*INCH])
        mirror([1, 0, 0])
            machinescrew(1.25 * INCH);
}

module panel() {
    color([0, 0.7, 0.9, 0.6])
        translate([-0.125*INCH, -.5 * PANEL_WIDTH,  -INCH])
            cube([0.125*INCH, PANEL_WIDTH, PANEL_HEIGHT + 2*INCH]);
    translate([0.65*INCH, 0, PANEL_HEIGHT + 0.5*INCH])
        machinescrew(1.25 * INCH);
    translate([0.65*INCH, 0, -0.5*INCH])
        machinescrew(1.75 * INCH);
}

module front() {
    cutout_width = PANEL_WIDTH;
    difference() {
        halfpipe();
        translate([0, -cutout_width / 2, PANEL_Z])
            cube([2 * INCH, cutout_width, PANEL_HEIGHT]);
    }
    keyboard();
    %translate([PANEL_X, 0, PANEL_Z])
        panel();
    %translate([1.75*INCH, 0, OVERALL_LENGTH - 1.5*INCH])
        machinescrew(1.75 * INCH);
}

module back() {
    mirror([1, 0, 0]) halfpipe();
    translate([-PLYWOOD_X, 0, PLYWOOD_Z])
        plywood();
    translate([-1.7*INCH, 0, 1.5*INCH])
        mirror([1, 0, 0])
            machinescrew(1.5 * INCH);
    translate([-1.7*INCH, 0, OVERALL_LENGTH - 1.5*INCH])
        mirror([1, 0, 0])
            machinescrew(1.5 * INCH);
}

%translate([SEPARATION / 2, 0, 0]) front();
translate([-SEPARATION / 2, 0, 0]) back();
