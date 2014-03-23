#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include "opencv2/nonfree/features2d.hpp"

using namespace cv;
using namespace std;

#define DICTIONARY_BUILD 1 // set DICTIONARY_BUILD to 1 for Step 1. 0 for step 2#if DICTIONARY_BUILD == 1

int main(int argc, char** argv)
{
	#if DICTIONARY_BUILD == 1
		//Step 1 - Obtain the set of bags of features.
		Mat input;    
		//To store the keypoints that will be extracted by SIFT
		vector<KeyPoint> keypoints;
		//To store the SIFT descriptor of current image
		Mat descriptor;
		//To store all the descriptors that are extracted from all the images.
		Mat featuresUnclustered;
		//The SIFT feature extractor and descriptor
		SiftDescriptorExtractor detector;

		//I select 20 (1000/50) images from 1000 images to extract
		//feature descriptors and build the vocabulary
		for(int h=1; h<argc; h+=5)
		{        
		    //open the file
		    input = imread(argv[h], CV_LOAD_IMAGE_GRAYSCALE); //Load as grayscale                
		    //detect feature points
		    detector.detect(input, keypoints);
		    //compute the descriptors for each keypoint
		    detector.compute(input, keypoints,descriptor);        
		    //put the all feature descriptors in a single Mat object 
		    featuresUnclustered.push_back(descriptor);        
		}    

		//Construct BOWKMeansTrainer
		//the number of bags
		int dictionarySize=200;
		//define Term Criteria
		TermCriteria tc(CV_TERMCRIT_ITER,100,0.001);
		//retries number
		int retries=1;
		//necessary flags
		int flags=KMEANS_PP_CENTERS;
		//Create the BoW (or BoF) trainer
		BOWKMeansTrainer bowTrainer(dictionarySize,tc,retries,flags);
		//cluster the feature vectors
		Mat dictionary=bowTrainer.cluster(featuresUnclustered);

		//store the vocabulary
		FileStorage fs("dictionary.yml", FileStorage::WRITE);
		fs << "vocabulary" << dictionary;
		fs.release();
	#else
	    //Step 2 - Obtain the BoF descriptor for given image/video frame. 

	    //prepare BOW descriptor extractor from the dictionary    
	    Mat dictionary; 
	    FileStorage fs("dictionary.yml", FileStorage::READ);
	    fs["vocabulary"] >> dictionary;
	    fs.release(); 

	    cout << dictionary.rows << " "<< dictionary.cols << endl;
	    
	    //create a nearest neighbor matcher
	    Ptr<DescriptorMatcher> matcher(new FlannBasedMatcher);
	    //create Sift feature point extracter
	    Ptr<FeatureDetector> detector(new SiftFeatureDetector());
	    //create Sift descriptor extractor
	    Ptr<DescriptorExtractor> extractor(new SiftDescriptorExtractor);    
	    //create BoF (or BoW) descriptor extractor
	    BOWImgDescriptorExtractor bowDE(extractor,matcher);
	    //Set the dictionary with the vocabulary we created in the first step
	    bowDE.setVocabulary(dictionary);
	 	
	 	map<string,map<string,int> > confusion_matrix;
	 	map<string,SVM> classes_classifiers;

	 	for(int h=1; h<argc; h++)
		{ 	         
		    //read the image
		    Mat img=imread(argv[h], CV_LOAD_IMAGE_GRAYSCALE);        
		    //To store the keypoints that will be extracted by SIFT
		    vector<KeyPoint> keypoints;        
		    //Detect SIFT keypoints (or feature points)
		    detector->detect(img,keypoints);
		    //To store the BoW (or BoF) representation of the image
		    Mat bowDescriptor;        
		    //extract BoW (or BoF) descriptor from given image

		    bowDE.compute(img,keypoints,bowDescriptor);

		    cout << argv[h] << endl;
			for(int i=0; i<bowDescriptor.cols; i++)
				cout << " " << bowDescriptor.at<float>(0, i) << " ";
			cout << endl;
		}
	#endif
}