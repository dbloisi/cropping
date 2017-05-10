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
// Original image
Mat img;
// Cropped image
Mat crop;

// Starting and ending points of the user's selection
Point corner1, corner2;
Point drag;
// ROI
Rect box;
//min clipping area width
int min_width;
//min clipping area height
int min_height;

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
	
    namedWindow("Cropping app");
    setMouseCallback("Cropping app", mouse_callback);

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
		
		
        imshow("Cropping app", img);
        
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
	if (corner2.x == corner1.x) {
	    corner2.x = corner1.x + min_width;
	}
	corner2.y = corner1.y + (x - corner1.x)*(min_width / min_height);
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
           Mat local_img = img.clone();
           rectangle(local_img, corner1, corner2, Scalar(0, 0, 255));
           imshow("Cropping app", local_img);
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
	
	    corner1.x -= (drag.x - x);
	    corner1.y -= (drag.y - y);

	    corner2.x -= (drag.x - x);
	    corner2.y -= (drag.y - y);

	    Mat local_img = img.clone();
			
	    rectangle(local_img, corner1, corner2, Scalar(0, 0, 255));
	    imshow("Cropping app", local_img);
	    waitKey(30);
	    drag.x = x;
	    drag.y = y;
	
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

#ifdef VIDEO_OK

	VideoCapture cap(video_name); // open the default camera
	if (!cap.isOpened()) {  // check if we succeeded
		cout << "Unable to open " << video_name << endl;
		cout << "Exiting..." << endl;
		exit(EXIT_FAILURE);
	}

	if (ms > 0) {
		cap.set(CV_CAP_PROP_POS_MSEC, ms);
	}

	int ex = static_cast<int>(cap.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form
	// Transform from int to char via Bitwise operators
	char EXT[] = { (char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0 };

	int fps = cap.get(CV_CAP_PROP_FPS);
	int width = (int)cap.get(CV_CAP_PROP_FRAME_WIDTH);
	int height = (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	
	Size S = Size(width, height);

	cout << "Input frame resolution: Width=" << S.width << "  Height=" << S.height
		<< " of nr#: " << cap.get(CV_CAP_PROP_FRAME_COUNT) << endl;
	cout << "Input codec type: " << EXT << endl;
	cout << "FPS = " << fps << endl;
	
	int interval = 0;
	bool run = true;

	int frame_number = cap.get(CV_CAP_PROP_FRAME_COUNT);

	while (run) {
		cap >> img;
	    if (!img.data) {
		    cerr << "Unable to load image from video " << video_name << endl;
		    run = false;
			break;
	    }

	    if (img.cols > 1024) {
		    is_resized = true;
		    resized_img.create(Size(img.cols / 2, img.rows / 2), CV_8UC3);
		    resize(img, resized_img, resized_img.size());
	    }



	    annotated = img.clone();



	    if(is_resized) {
		    resized_annotated.create(Size(annotated.cols / 2, annotated.rows / 2), CV_8UC3);
		    resize(annotated, resized_annotated, resized_annotated.size());
	    }

	    if (is_resized) {
		    namedWindow("Cropping app (RESIZED)");
		    imshow("Cropping app (RESIZED)", resized_annotated);
		    waitKey(interval);
	    }
	    else {
		    namedWindow("Cropping app");
		    imshow("Cropping app", annotated);
		    waitKey(interval);
	    }


	    // Set the mouse event callback function
	    if (is_resized) {
		    setMouseCallback("Cropping app (RESIZED)", mouse_callback);
	    }
	    else {
		    setMouseCallback("Cropping app", mouse_callback);
	    }

	    // Exit by pressing 'q' or esc
	    
		char key = char(waitKey(interval));
		if (key == 'q' || key == 27) {
			run = false;
		}
		else if (key == 'p') {
			interval = 0;
		}
		else if (key == 'f') {
			interval = 30;
		}
		else if (key == 's') {

			bool saving = true;
			bool saved = false;
			bool img_stored = false;

			ofstream obs_file;
			
			while (saving) {
				//cout << "saving crop image" << endl;

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

					if (is_annotated) {
						Annotation current_ann;
						current_ann.bounding_box.x = -1000;
						float min_dist = 0.f;
						for (std::vector<Annotation>::iterator it = annotations.begin(); it != annotations.end(); ++it) {
							if (current_ann.bounding_box.x == -1000) {
								current_ann = *it;
								min_dist = sqrt(pow(current_ann.bounding_box.x - box.x, 2) + pow(current_ann.bounding_box.y - box.y, 2));
							}
							else {
								float dist = sqrt(pow((*it).bounding_box.x - box.x, 2) + pow((*it).bounding_box.y - box.y, 2));
								if (dist < min_dist) {
									current_ann = *it;
									min_dist = sqrt(pow(current_ann.bounding_box.x - box.x, 2) + pow(current_ann.bounding_box.y - box.y, 2));
								}
							}

						}


						saved_crop_name.assign(video_name.substr(0, idx) + "_frame_" + n + "_crop_" + box_value + "-" + current_ann.class_name + ".png");

					}
					else {

						//saved_crop_name.assign(video_name.substr(0, idx) + "_frame_" + n + "_crop_" + box_value + ".png");
						saved_crop_name.assign(crop_dir + "epl_LEILIV_frame_" + n + "_crop_" + box_value + ".png");

						if (!img_stored) {
							saved_image_name.assign(whole_dir + "epl_LEILIV_frame_" + n + ".png");
							
							string obs_name = whole_dir + "epl_LEILIV_frame_" + n + ".txt";
							cout << "creating file " << obs_name << endl;
							obs_file.open(obs_name);
							if (!obs_file.is_open())
							{
								cout << "unable to save object annotations." << endl;
								exit(EXIT_FAILURE);
							}
							else {
								cout << "file " << obs_name << "created." << endl;
							}
						}

					}

					if (imwrite(saved_crop_name, crop)) {
						cout << "crop: " << saved_crop_name << " saved." << endl;
					}
					else {
						cerr << "Unable to save image: " << saved_crop_name << endl;
						exit(EXIT_FAILURE);
					}

					ostringstream ss10;
					ss10 << box.tl().x;
					ostringstream ss11;
					ss11 << box.tl().y;
					ostringstream ss12;
					ss12 << box.width;
					ostringstream ss13;
					ss13 << box.height;
					string obj_value = "player " + ss10.str() + " " + ss11.str() + " " + ss12.str() + " " + ss13.str();
					obs_file << obj_value << endl;

					if (!img_stored) {

						if (imwrite(saved_image_name, img)) {
							cout << "frame: " << saved_image_name << " saved." << endl;
						}
						else {
							cerr << "Unable to save frame: " << saved_image_name << endl;
							exit(EXIT_FAILURE);
						}
						img_stored = true;

					}

					saved = true;
				}

				key = char(waitKey(30));
				if (key == 'n') {
					saving = false;
					obs_file.close();
				}
				else if (key == 's') {
					saved = false;
				}
			}

		}

        frame_number++;

	}
#endif
}

