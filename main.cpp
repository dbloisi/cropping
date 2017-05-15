#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <stdlib.h>
#include <math.h>

#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "imagemanager.h"

using namespace std;
using namespace cv;

// Global variables
// Flags updated according to left mouse button activity
bool ldown = false, lup = false;
// Flags updated according to right mouse button activity
bool rdown = false, rup = false;
// current image
Mat img;
Mat prev_img;
// Cropped image
Mat crop;
Mat gui_frame;

// Starting and ending points of the user's selection
Point corner1, corner2;
Point drag;
// ROI
Rect box;
//min clipping area width
int min_width;
//min clipping area height
int min_height;

bool is_resized;
const int MAX_WIDTH = 1024;

string window_name;

//function headers
void help();
static void mouse_callback(int event, int x, int y, int, void *);

void processDir(string dir_name);

void processVideo(string video_name, int ms);

string crop_dir = "cropped/";

string whole_dir = "whole/";


/**
* @function help
*/
void help()
{
    cout
    << "--------------------------------------------------------------------------" << endl
    << "Usage:"                                                                     << endl
    << "crop /{-vid <video filename> | -dir <folder name>/} -w <cropped area width> -h <cropped area height> [-ms <n milliseconds>]"<< endl
    << "examples:"                                                                  << endl
    << "crop -dir data/images -w 60 -h 30"                                          << endl
    << "crop -vid video.avi -w 64 -h 128"                                           << endl
    << "crop -vid video.avi -w 64 -h 128 -ms 600000"                                << endl
    << "--------------------------------------------------------------------------" << endl
                                                                                    << endl;
}


//main function
int main(int argc, char* argv[])
{
    //print help information
    help();
	
    string dir_name;
    bool is_dir = false;
    bool is_video = false;
    int ms = -1;
	
    //check for the input parameter correctness
    if(argc < 7) {
        cerr <<"Incorrect input list" << endl;
        cerr <<"exiting..." << endl;
        return EXIT_FAILURE;
    }

    string image_name;
    string video_name;
    
    for(int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-vid") == 0) {
	    video_name.assign(argv[++i]);
	    is_video = true;
	}
	else if(strcmp(argv[i], "-dir") == 0) {
	    dir_name.assign(argv[++i]);
	    is_dir = true;
        }				
        else if(strcmp(argv[i], "-w") == 0) {
            min_width = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-h") == 0) {
            min_height = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-ms") == 0) {
	    ms = atoi(argv[++i]);
	}
        else {
            //error in reading input parameters
            cerr <<"Please, check the input parameters." << endl;
            cerr <<"Exiting..." << endl;
            return EXIT_FAILURE;
        }
    }
	
    if (is_video) {
        processVideo(video_name, ms);
    }
    else {
        if(is_dir) {
	        processDir(dir_name);
	    }
    }

    return EXIT_SUCCESS;
}
	
void processDir(string dir_name) {		
		
    ImageManager *im = new ImageManager(dir_name);
		
    int keyboard = -1;
	
	window_name.append("Cropping app");
    namedWindow(window_name);
    setMouseCallback(window_name, mouse_callback);

    //read input data. ESC or 'q' for quitting
    while( (char)keyboard != 'q' && (char)keyboard != 27 )
    {		
	    box.x = 0;
        box.y = 0;
        box.width = min_width;
        box.height = min_height;

	//read the first file of the sequence
        string image_name = im->next(1);
		
        cout << "reading image: " << image_name << endl;
        img = imread(image_name);

		
	if(!img.data) {
	    //error in opening the first image
            cerr << "Unable to open first image frame: " << image_name << endl;
	    exit(EXIT_FAILURE);
	}

	cout << "W: " << img.cols << "   H: " << img.rows << endl;
		
		
        imshow(window_name, img);

		gui_frame = img.clone();
        
        keyboard = waitKey(0);
		
        if(keyboard == 's') {

            ostringstream ss1;
            ss1 << box.tl().x;
            ostringstream ss2;
            ss2 << box.tl().y;
            ostringstream ss3;
            ss3 << box.br().x;
            ostringstream ss4;
            ss4 << box.br().y;
            string box_value = ss1.str()+"_"+ss2.str()+"_"+ss3.str()+"_"+ss4.str();
            int idx = image_name.find_last_of("/");
            image_name = image_name.substr(idx+1);
            idx = image_name.find_last_of(".");

            string saved_crop_name;
            
            saved_crop_name.assign(image_name.substr(0,idx) + "_crop_" + box_value + ".png");


            if(box.x == 0 && box.y == 0) {
                crop = img.clone();
            }

            if(imwrite(saved_crop_name, crop)) {
                cout << "image: " << saved_crop_name  << " saved." << endl;
            }
            else {
                cerr << "Unable to save image: " << saved_crop_name  << endl;
				exit(EXIT_FAILURE);
            }

        }
		     
    }	
	    
}

// Callback function for mouse events
static void mouse_callback(int event, int x, int y, int, void *)
{
    // When the left mouse button is pressed, record its position and save it in corner1
    if(event == EVENT_LBUTTONDOWN)
    {
        ldown = true;
        corner1.x = x;
        corner1.y = y;	        
    }
    // When the left mouse button is released, record its position and save it in corner2
    if(event == EVENT_LBUTTONUP)
    {
        lup = true;
    }
    // Update the box showing the selected region as the user drags the mouse
    if(ldown == true && lup == false)
    {
        corner2.x = x;
		float def_ratio = (min_width / min_height);
		//(corner2.x - corner1.x) : (corner2.y - corner1.y) = def_ratio
		//(corner2.x - corner1.x) / def_ratio = (corner2.y - corner1.y)
		corner2.y = cvRound((corner2.x - corner1.x) / def_ratio) + corner1.y;
        if (corner2.y == corner1.y) {
			corner2.y = corner1.y + min_height;
        }

        if(0 <= box.x &&
           0 <= box.width &&
           box.x + box.width <= img.cols &&
           0 <= box.y &&
           0 <= box.height &&
           box.y + box.height <= img.rows)
        {
			Mat local_img;
            if (gui_frame.data) {
				local_img = gui_frame.clone();
			}
			else {
                local_img = img.clone();
			}
            rectangle(local_img, corner1, corner2, Scalar(0, 0, 255));
            imshow(window_name, local_img);
        }     
    }

    // Define ROI and crop it out when both corners have been selected
    if(ldown == true && lup == true)
    {
        box.width = abs(corner1.x - corner2.x);
        box.height = abs(corner1.y - corner2.y);
        box.x = min(corner1.x, corner2.x);
        box.y = min(corner1.y, corner2.y);
        if(0 <= box.x &&
           0 <= box.width &&
           box.x + box.width <= img.cols &&
           0 <= box.y &&
           0 <= box.height &&
           box.y + box.height <= img.rows)
        {
            Mat crop_current(img, box);
			if(crop_current.size().width > 0 && crop_current.size().height > 0) {
                namedWindow("Crop");
                imshow("Crop", crop_current);
                crop = crop_current.clone();
            }		
        }		
        ldown = false;
        lup = false;        
    }

    if(event == EVENT_RBUTTONDOWN)
    {
        rdown = true;
        drag.x = x;
        drag.y = y;
    }


    if(event == EVENT_RBUTTONUP)
    {
        rup = true;
    }

    // Update the box showing the selected region as the user drags the mouse
    if(rdown == true && rup == false)
    {	
		int current_width = std::abs(corner1.x - corner2.x);
		int current_height = std::abs(corner1.y - corner2.y);
	    int offset_x = (x - drag.x);
	    int offset_y = (y - drag.y);

		corner1.x += offset_x;
		corner1.y += offset_y;

		corner2.x = corner1.x + current_width;
		corner2.y = corner1.y + current_height;

		drag.x = x;
		drag.y = y;

		Mat local_img;
		if (gui_frame.data) {
			local_img = gui_frame.clone();
		}
		else {
			local_img = img.clone();
		}
	    rectangle(local_img, corner1, corner2, Scalar(0, 0, 255));
	    imshow(window_name, local_img);
	    waitKey(30);
	    
	
    }


    if(rdown == true && rup == true)
    {
        box.width = abs(corner1.x - corner2.x);
        box.height = abs(corner1.y - corner2.y);
        box.x = min(corner1.x, corner2.x);
        box.y = min(corner1.y, corner2.y);

        // Make an image out of just the selected ROI and display it in a new window
		
			
            if(0 <= box.x && 0 < box.width &&
				(box.x + box.width) <= img.cols &&
				0 <= box.y && 0 < box.height &&
				(box.y + box.height) <= img.rows)
			{
				Mat crop_current(img, box);
				namedWindow("Crop");
				imshow("Crop", crop_current);
				waitKey(30);
				crop = crop_current.clone();
			}
		
		rdown = false;
        rup = false;
    }


    if(ldown == false && lup == true) {
        lup = false;
    }
    if(rdown == false && rup == true) {
        rup = false;
    }
 }


void processVideo(string video_name, int ms) {

	VideoCapture cap(video_name);
	if (!cap.isOpened()) {
		cout << "Unable to open " << video_name << endl;
		cout << "Exiting..." << endl;
		exit(EXIT_FAILURE);
	}

	if (ms > 0) {
		cap.set(CV_CAP_PROP_POS_MSEC, ms);
	}

	int ex = static_cast<int>(cap.get(CV_CAP_PROP_FOURCC));
	char EXT[] = { (char)(ex & 0XFF), (char)((ex & 0XFF00) >> 8),
		           (char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0 };

	int fps = cap.get(CV_CAP_PROP_FPS);
	int width = (int)cap.get(CV_CAP_PROP_FRAME_WIDTH);
	int height = (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	
	Size S = Size(width, height);

	cout << "Input video resolution: Width=" << S.width << "  Height=" << S.height
		<< " of nr#: " << cap.get(CV_CAP_PROP_FRAME_COUNT) << endl;
	cout << "Input codec type: " << EXT << endl;
	cout << "FPS = " << fps << endl;
	
    //init
	int interval = 0;
	bool run = true;
	int frame_number = cap.get(CV_CAP_PROP_FRAME_COUNT);
		
	cap >> img;
	if (!img.data) {
		cerr << "Unable to load image from video " << video_name << endl;
		exit(EXIT_FAILURE);
	}

	if (img.cols > MAX_WIDTH) {
		is_resized = true;
		window_name.append("Cropping app (RESIZED)");
	}
	else {
		window_name.append("Cropping app");
	}
	namedWindow(window_name);
	setMouseCallback(window_name, mouse_callback);

	while (run) {
		cap >> img;
	    if (!img.data) {
		    cerr << "Unable to load image from video " << video_name << endl;
		    run = false;
			break;
	    }

	    if (is_resized) {
		    Mat resized_img(Size(img.cols / 2, img.rows / 2), CV_8UC3);
		    resize(img, resized_img, resized_img.size());
			img = resized_img.clone();
	    }
	    
		imshow(window_name, img);

		gui_frame = img.clone();
		    
		char key = char(waitKey(interval));
		if (key == 'q' || key == 27) {
			run = false;
		}
		else if (key == 'b') { //block
			interval = 0;
		}
		else if (key == 'p') { //play
			interval = 30;
		}
		else if (key == 's') { //save

			if (interval != 0) {
				interval = 0;
			}

			bool saving = true;
			bool saved = false;
			bool img_stored = false;

			ofstream obs_file;

			gui_frame = img.clone();
			
			while (saving) {

				if (!saved) {

					ostringstream ss1;
					ss1 << box.tl().x;
					ostringstream ss2;
					ss2 << box.tl().y;
					ostringstream ss3;
					ss3 << box.br().x;
					ostringstream ss4;
					ss4 << box.br().y;
					string box_value = ss1.str() + "_" + ss2.str() + "_" + ss3.str() + "_" + ss4.str();

					int idx = video_name.find_last_of(".");

					ostringstream ss5;
					ss5 << frame_number;
					string n = ss5.str();

					string saved_crop_name;
					string saved_image_name;

					saved_crop_name.assign(video_name.substr(0, idx) + "_frame_" + n + "_crop_" + box_value + ".png");
					
					
					if (imwrite(saved_crop_name, crop)) {
						cout << "crop: " << saved_crop_name << " saved." << endl;
					}
					else {
						cerr << "Unable to save image: " << saved_crop_name << endl;
						exit(EXIT_FAILURE);
					}

					saved = true;

					rectangle(gui_frame, box, Scalar(0, 255, 0), 2);
					imshow(window_name, gui_frame);

				}

				key = char(waitKey(30));
				if (key == 'n') {
					saving = false;
					obs_file.close();
					gui_frame = img.clone();
				}
				else if (key == 's') {
					saved = false;
				}
			}

		}

        frame_number++;

	}
}

