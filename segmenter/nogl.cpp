#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <string.h>
#include <iostream>
#include <sys/time.h>
using namespace cv;
using namespace std;

// Global variables
Mat src, dilation_dst,dst,binary,close,dst1;
int scale=2;
//stuctiring element
Mat element = getStructuringElement( MORPH_ELLIPSE,
                                     	Size( 70,//horizontal
					25 ),// for vertical
                                       Point( -1, -1 ) );
// initialize bounding box for all cc as global variable
vector <Rect> bbox;//function op current!
vector <Rect> Mainbox;//overall box cordinate set by main function
vector < vector<Point2i > > blobs;// 1st vect for each cc and 2nd vec for all xy in that cc
int t_start,t_stop;
/** Function Headers */
void NonOverlap();
//declairing finding blob function
void FindBlobs(const Mat &binary);
// finding no of holes
int Holecout(int j);
/** @function main */
int main( int argc, char** argv )
{
  /// Load an image
	t_start= clock();
	int label;
  	Mat readed = imread( argv[1]),src1;
	if (readed.type()==CV_8UC1) 
       //input image is grayscale
    		cvtColor(readed, src1, CV_GRAY2RGB);
	else src1=readed;
	cvtColor(src1,src,CV_RGB2GRAY);
	subtract(Scalar::all(255),src,dst1);
  	if( !src.data )
  		{ return -1; }
//Downlampling for fast calculation
	pyrDown( dst1, dst, Size( dst1.cols/scale, dst1.rows/scale ) );
//area closing
	morphologyEx( dst, close, MORPH_CLOSE, element );
// thresholding
	threshold(close, binary, 0, 1, CV_THRESH_BINARY | CV_THRESH_OTSU);
// finding cc
	FindBlobs(binary);
	cout<<"initial no of bloks= "<<bbox.size()<<endl;
//function to remove overlaping bloks!
	NonOverlap();
	scale = 1;
	int nb;
	for (int i=0;i<7;i++)
	{
		nb = bbox.size();
		NonOverlap();
		if (nb==bbox.size())
			break;
	}
		cout<<"final no of bloks = "<<bbox.size()<<endl;

// from here onwords the function used should be used only mainbox!
	Mainbox = bbox;
// adding alpha channel to show the op

	IplImage* img = new IplImage(src1);
	IplImage* tmp;
	cv::Rect roi;
	cv:: Mat crop;
    
	//Mat alphaMask = Mat::zeros(src1.rows, src1.cols, CV_8UC3),alpOp;	
      	for(size_t i=0; i < Mainbox.size(); i++)
        {
                // rectangle(
                // alphaMask,
                // Rect (Mainbox[i]),
                // CV_RGB(255,0,0),
               	// -1);

                roi.x=Mainbox[i].x;
                roi.y=Mainbox[i].y;
                roi.width=Mainbox[i].width;
                roi.height=Mainbox[i].height;
                crop=src1(roi);
                
                std::string img_path = "./applications/Likhawat/segmenter/segmented_images/" + std::to_string(i) + ".jpg";

                imwrite(img_path.c_str(),crop);
                cout<<img_path<<"image write success"<<endl;
\
		// cout<<Mainbox[i].width<<endl;

		// putText(src1,s, cvPoint(Mainbox[i].x,Mainbox[i].y), 
  //   		FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(0,255,0), 2, CV_AA);
        }
	// adding alphachannel	
	// addWeighted( src1, .7, alphaMask, 0.3, 0.0,alpOp);
	// imwrite("final.jpg",alpOp);
	t_stop = clock();
	float time=((float)(t_stop - t_start))/CLOCKS_PER_SEC;
	cout<<"total execution time = "<<time<<endl;
/*	
	for(int c=0; c < bbox.size(); c++)
	{
		label =  Holecout(c);
		cout <<"Table ="<<label<<" of block no = "<<c<<endl;
	}
*/
	// label =  Holecout(4);
  	return 0;
}

void NonOverlap()
{
//function to remove overlaping bloks!
// initialize an image with zeros
        Mat non_overlap = Mat::zeros(dst1.rows, dst1.cols, CV_8UC1);
	cout<<"rows= "<<dst1.rows<<endl;
	cout<<"cols= "<<dst1.cols<<endl;
        for(size_t i=0; i < bbox.size(); i++)
        {
                rectangle(
                non_overlap,
              	cvPoint(bbox[i].x*scale,bbox[i].y*scale),
              	cvPoint((bbox[i].x+bbox[i].width)*scale,(bbox[i].y+bbox[i].height)*scale),
                CV_RGB(255,255,255),
                -1);
        }
        Mat binary1;
        threshold(non_overlap, binary1, 0,1, CV_THRESH_BINARY | CV_THRESH_OTSU);
        FindBlobs(binary1);

}
void FindBlobs(const Mat &binary)
{

// the two inportant out put of the function
        blobs.clear();
        bbox.clear();

        // Fill the label_image with the blobs
        // 0  - background
        // 1  - unlabelled foreground
        // 2+ - labelled foreground
//initializing a 32 bit public static final int CV_32SC1 
// since the labeling may exeed the limit 255
        Mat label_image;
        binary.convertTo(label_image, CV_32SC1);

// starts at 2 because 0,1 are used already
        int label_count = 2;

// starting the labeling process
//for each row
        for(int y=0; y < label_image.rows; y++)
        {

// call each row to array row
                int *row = (int*)label_image.ptr(y);

//for each column
                for(int x=0; x < label_image.cols; x++)
                {

// we have to label only forground pixel ie img(x,y)=1 else skip
                        if(row[x] != 1)
                        {
                                continue;
                        }

// rect for bounding box of the curresponding connected component
                Rect rect;
// flood fill is the function to find the connecting component areas
                floodFill(label_image,// input/output image
                        Point(x,y),// STARTING POIT /seed point
                        label_count, // filled with the vale=label
                        &rect, // bounding box of the cc
                        0, 0, 8);// uper threshold,lower thesh and 8 connectivity
// not need all the cordinates in cc
// initialize blob for all x,y values in the connected component
                vector <Point2i> blob;
// writing each xy cordinale to blobs and rect to bbox=bounding box
                for(int i=rect.y; i < (rect.y+rect.height); i++)
                {
                        int *row2 = (int*)label_image.ptr(i);
                        for(int j=rect.x; j < (rect.x+rect.width); j++)
                        {
                                if(row2[j] != label_count)
                                {
                                        continue;
                                }

                                blob.push_back(cv::Point2i(j,i));
                        }
                }

                blobs.push_back(blob);

                bbox.push_back(rect);
                label_count++;
                }
        }
}
// finding no of holes in A connected component
int Holecout(int j)
{
	int out=0; // 0 indicating the block i have no table!
// crop the original image
	Mat croppedImage = dst1(Mainbox[j]),cpd_with_boder,ccpd_with_boder;
//padding 
	copyMakeBorder( croppedImage, cpd_with_boder, 5,5,5,5, BORDER_CONSTANT, 0 );
	// finding countours
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours( cpd_with_boder, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	// Drowing function

	vector <int> parentList;
	for(int p=0;p<hierarchy.size();p++)
	{
// the connected componebt with no child and parrrent
		if (hierarchy[p][2]==-1 && hierarchy[p][3]>0)
// save the parrent name to list
			parentList.push_back(hierarchy[p][3]);	
	}
// finding the number of child for each parrent have
        vector <Point2i> childCount;
	for(int k=0;k<parentList.size();k++)
	{
	//	cout<< "parent list = "<<parentList[k]<<endl;
		int temp=parentList[k],su=0;
		for(int a=0;a<parentList.size();a++)
		{
			if(temp==parentList[a])
			su++;
		}
                childCount.push_back(cv::Point2i(temp,su));
	}
// if any parrent have 6 or more child the block declair as table!
	int max_count=0;
	for(int b=0;b<childCount.size();b++)
	{
//		cout<<childCount[b]<< endl;
		if(max_count< childCount[b].y)
			max_count= childCount[b].y;
		out=max_count;
	}
//  horizontal line detection!
//	Rect r= boundingRect(contours[172]);
//	rectangle(contours_img,Point(r.x,r.y), Point(r.x+r.width,r.y+r.height), Scalar(0,0,255),3,8,0);
	cvtColor(cpd_with_boder, ccpd_with_boder, CV_GRAY2BGR);

        Mat lineDrow = Mat::zeros(ccpd_with_boder.rows, ccpd_with_boder.cols, CV_8UC1);
	vector<Vec4i> lines;
  	HoughLinesP(cpd_with_boder, lines, 1, 3.1415/2, 100, 500, 10 );
  	for( size_t i = 0; i < lines.size(); i++ )
  	{
    		Vec4i l = lines[i];
    		line( lineDrow, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(1), 3, CV_AA);
  	}
//	finding blobs
	FindBlobs(lineDrow);
	int noStraitLine=0;
	for( int i = 0; i < bbox.size(); i++ )
	{
		if (bbox[i].height<10 && bbox[i].width>Mainbox[j].width*0.75)
			noStraitLine++;
	}
		cout<<"No: straightlines  "<<noStraitLine<<endl;	
//	namedWindow( "Display window", WINDOW_NORMAL );// Create a window for display.
//	imshow( "Display window",lineDrow );                   // Show our image inside it.
//	waitKey(0);
/*
         vector <Rect> parallelAnalysys;
         for( size_t i = 0; i < lines.size(); i++ )
         {
                 Vec4i l1 = lines[i];
                 for( size_t j = 0; j < lines.size(); j++ )
                 {
                         Vec4i l2 = lines[j];
                         if(i==j )
                                 continue;
                         else 
                         {
                                 Rect temp;
                                 temp.x = i;
                                 temp.y = j;
                                 temp.width = (l1[1]-l1[3])*(l1[1]-l1[3])+(l2[1]-l2[3])*(l2[1]-l2[3])+ //for horizontal condition
                                 (l1[0]-l2[0])*(l1[0]-l2[0])+(l1[2]-l2[2])*(l1[2]-l2[2]); // for equal length
                                 temp.height = (l1[1]-l2[3])*(l1[1]-l2[3]); 
                                 if (temp.height>1000)
                                         parallelAnalysys.push_back(temp);                               
                         }
                 }
         }

 */
//	cout<<hierarchy[288][3]<<" hierachy"<<endl;
//	cout<<blobs[j][0].y<<" blobs"<<endl;
  	return out;
}
