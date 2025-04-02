/*
  Copyright ©2013 The Regents of the University of California
  (Regents). All Rights Reserved. Permission to use, copy, modify, and
  distribute this software and its documentation for educational,
  research, and not-for-profit purposes, without fee and without a
  signed licensing agreement, is hereby granted, provided that the
  above copyright notice, this paragraph and the following two
  paragraphs appear in all copies, modifications, and
  distributions. Contact The Office of Technology Licensing, UC
  Berkeley, 2150 Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620,
  (510) 643-7201, for commercial licensing opportunities.

  IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT,
  INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
  LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
  DOCUMENTATION, EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY
  OF SUCH DAMAGE.

  REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
  DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS
  IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
  UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#include "runphysics.hpp"

#include "conf.hpp"
#include "io.hpp"
#include "misc.hpp"
#include "separateobs.hpp"
#include "simulation.hpp"
#include "timer.hpp"
#include "util.hpp"
#include <boost/filesystem.hpp>
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <iomanip>

using namespace std;

string inprefix, outprefix;
static fstream timingfile;
bool export_obstacles = true; // Default to exporting obstacles
bool structured_output = true; // Structured, parsable output format

Simulation sim;
int frame;
Timer fps;

void copy_file (const string &input, const string &output);

void print_status(const string& status, const string& details = "") {
    if (structured_output) {
        if (details.empty()) {
            cout << "{ \"status\": \"" << status << "\" }" << endl;
        } else {
            cout << "{ \"status\": \"" << status << "\", \"details\": \"" << details << "\" }" << endl;
        }
    } else {
        if (details.empty()) {
            cout << status << endl;
        } else {
            cout << status << ": " << details << endl;
        }
    }
}

void print_frame_progress(int frame, int step, double time) {
    if (structured_output) {
        cout << "{ \"frame\": " << frame 
             << ", \"step\": " << step 
             << ", \"time\": " << fixed << setprecision(6) << time
             << " }" << endl;
    } else {
        cout << "Sim frame " << frame << " [" << step << "]" << endl;
    }
}

void init_physics (const string &json_file, string outprefix,
                   bool is_reloading) {
    print_status("INIT_PHYSICS_START", json_file);
    
    load_json(json_file, sim);
    ::outprefix = outprefix;
    if (!outprefix.empty()) {
        ::timingfile.open(stringf("%s/timing", outprefix.c_str()).c_str(),
                          is_reloading ? ios::out|ios::app : ios::out);
        // Make a copy of the config file for future use
        copy_file(json_file.c_str(), stringf("%s/conf.json",outprefix.c_str()));
        // And copy over all the obstacles
        vector<Mesh*> base_meshes(sim.obstacles.size());
        for (int o = 0; o < (int)sim.obstacles.size(); o++)
            base_meshes[o] = &sim.obstacles[o].base_mesh;
        save_objs(base_meshes, stringf("%s/obs", outprefix.c_str()));
    }
    prepare(sim);
    
    print_status("INIT_PHYSICS_COMPLETE");
}

void init_relax() {
    print_status("INIT_RELAX_START");
    
    separate_obstacles(sim.obstacle_meshes, sim.cloth_meshes);
    relax_initial_state(sim);
    
    print_status("INIT_RELAX_COMPLETE");
}

static void save (vector<Mesh*> &meshes, int frame) {
    if (!outprefix.empty() && frame < 100000) {
        print_status("SAVE_START", "frame=" + to_string(frame));
        
        // Save binary state file
        string binfile = stringf("%s/%05d", outprefix.c_str(), frame);
        // Create directory if it doesn't exist
        boost::filesystem::path parent_path = boost::filesystem::path(binfile).parent_path();
        if (!boost::filesystem::exists(parent_path)) {
            boost::filesystem::create_directories(parent_path);
        }
        save_state(sim, binfile);
        
        // Save OBJ files directly
        for (int c = 0; c < (int)meshes.size(); c++) {
            string objfile = stringf("%s/cloth%d_frame%d.obj", outprefix.c_str(), c, frame);
            save_obj(*meshes[c], objfile);
            
            if (structured_output) {
                cout << "{ \"saved\": \"cloth" << c << "\", \"frame\": " << frame 
                     << ", \"file\": \"" << objfile << "\" }" << endl;
            }
        }
        
        // Save obstacle OBJs too, if enabled
        if (export_obstacles) {
            for (int o = 0; o < (int)sim.obstacles.size(); o++) {
                string objfile = stringf("%s/obstacle%d_frame%d.obj", outprefix.c_str(), o, frame);
                save_obj(sim.obstacles[o].get_mesh(), objfile);
                
                if (structured_output) {
                    cout << "{ \"saved\": \"obstacle" << o << "\", \"frame\": " << frame 
                         << ", \"file\": \"" << objfile << "\" }" << endl;
                }
            }
        }
        
        print_status("SAVE_COMPLETE", "frame=" + to_string(frame));
    }
}

static void save_timings () {
    static double old_totals[Simulation::nModules] = {};
    if (!::timingfile)
        return; // printing timing data to stdout is getting annoying
    ostream &out = ::timingfile ? ::timingfile : cout;
    for (int i = 0; i < Simulation::nModules; i++) {
        out << sim.timers[i].total - old_totals[i] << " ";
        old_totals[i] = sim.timers[i].total;
    }
    out << endl;
}

void save (Simulation &sim, int frame) {
    save(sim.cloth_meshes, frame);
}

void sim_step() {
    fps.tick();
    advance_step(sim);
    if ((sim.step % sim.frame_steps == 0) && (sim.step % sim.save_every) == 0) {
        print_frame_progress(sim.frame, sim.step, sim.time);
        save(sim, sim.frame);
        save_timings();
    } else {
        if (structured_output && sim.step % 10 == 0) {
            // Print progress less frequently to avoid too much output
            print_frame_progress(sim.frame, sim.step, sim.time);
        }
    }
    fps.tock();
    if (sim.time >= sim.end_time || sim.frame >= sim.end_frame) {
        print_status("SIMULATION_COMPLETE", "time=" + to_string(sim.time) + 
                     ", frames=" + to_string(sim.frame));
        exit(EXIT_SUCCESS);
    }
}

void offline_loop() {
    print_status("SIMULATION_START", "end_time=" + to_string(sim.end_time) + 
                 ", end_frame=" + to_string(sim.end_frame));
    
    while (true)
        sim_step();
}

void run_physics (const vector<string> &args) {
    // Check for formatting flags
    bool found_flag = false;
    vector<string> filtered_args;
    for (const string &arg : args) {
        if (arg == "--no-export-obstacles") {
            export_obstacles = false;
            found_flag = true;
        } else if (arg == "--raw-output") {
            structured_output = false;
            found_flag = true;
        } else {
            filtered_args.push_back(arg);
        }
    }
    
    if (filtered_args.size() != 1 && filtered_args.size() != 2) {
        cout << "Runs the simulation in batch mode." << endl;
        cout << "Arguments:" << endl;
        cout << "    <scene-file>: JSON file describing the simulation setup" << endl;
        cout << "    <out-dir> (optional): Directory to save output in" << endl;
        cout << "    --no-export-obstacles (optional): Skip exporting obstacle OBJs" << endl;
        cout << "    --raw-output (optional): Use simple output format instead of JSON" << endl;
        exit(EXIT_FAILURE);
    }
    
    string json_file = filtered_args[0];
    string outprefix = filtered_args.size() > 1 ? filtered_args[1] : "";
    
    print_status("CONFIG", "json_file=" + json_file + 
                 ", output_dir=" + (outprefix.empty() ? "none" : outprefix) + 
                 ", export_obstacles=" + (export_obstacles ? "true" : "false"));
    
    if (!outprefix.empty()) {
        // Create output directory if it doesn't exist
        try {
            boost::filesystem::path dir(outprefix);
            if (!boost::filesystem::exists(dir)) {
                boost::filesystem::create_directories(dir);
                print_status("DIRECTORY_CREATED", outprefix);
            }
        } catch (const boost::filesystem::filesystem_error& e) {
            print_status("ERROR", string("Failed to create directory: ") + e.what());
            exit(EXIT_FAILURE);
        }
        ensure_existing_directory(outprefix);
    }
    
    init_physics(json_file, outprefix, false);
    init_relax();
    if (!outprefix.empty())
        save(sim, 0);
    offline_loop();
}

void init_resume(const vector<string> &args) {
    assert(args.size() == 2);
    string outprefix = args[0];
    string start_frame_str = args[1];
    
    print_status("RESUME_START", "from_frame=" + start_frame_str);
    
    // Load like we would normally begin physics
    init_physics(stringf("%s/conf.json", outprefix.c_str()), outprefix, true);
    // Get the initialization information
    sim.frame = atoi(start_frame_str.c_str());
    for (int i=0; i<sim.frame; i++)
    	sim.time += sim.frame_time;
    sim.step = sim.frame * sim.frame_steps;
    for(int i=0; i<(int)sim.obstacles.size(); ++i)
        sim.obstacles[i].get_mesh(sim.time);
    prepare(sim); // set cloth meshes etc.
    load_state(sim, stringf("%s/%05d",outprefix.c_str(),sim.frame));
    //separate_obstacles(sim.obstacle_meshes, sim.cloth_meshes);
    
    print_status("RESUME_COMPLETE", "time=" + to_string(sim.time));
}

void resume_physics (const vector<string> &args) {
    // Check for formatting flags
    bool found_flag = false;
    vector<string> filtered_args;
    for (const string &arg : args) {
        if (arg == "--no-export-obstacles") {
            export_obstacles = false;
            found_flag = true;
        } else if (arg == "--raw-output") {
            structured_output = false;
            found_flag = true;
        } else {
            filtered_args.push_back(arg);
        }
    }
    
    if (filtered_args.size() != 2) {
        cout << "Resumes an incomplete simulation in batch mode." << endl;
        cout << "Arguments:" << endl;
        cout << "    <out-dir>: Directory containing simulation output files" << endl;
        cout << "    <resume-frame>: Frame number to resume from" << endl;
        cout << "    --no-export-obstacles (optional): Skip exporting obstacle OBJs" << endl;
        cout << "    --raw-output (optional): Use simple output format instead of JSON" << endl;
        exit(EXIT_FAILURE);
    }
    
    print_status("CONFIG", "output_dir=" + filtered_args[0] + 
                 ", resume_frame=" + filtered_args[1] + 
                 ", export_obstacles=" + (export_obstacles ? "true" : "false"));
    
    init_resume(filtered_args);
    offline_loop();
}

void copy_file (const string &input, const string &output) {
    return;
    if(input == output) {
        return;
    }
    if(boost::filesystem::exists(output)) {
        boost::filesystem::remove(output);
    }
    boost::filesystem::copy_file(
        input, output);
}
