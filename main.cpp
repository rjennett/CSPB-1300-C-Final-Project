/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Ryan Jennett

- All project requirements fully met? (YES or NO):
    <ANSWER>

- If no, please explain what you could not get to work:
    <ANSWER>

- Did you do any optional enhancements? If so, please explain:
    <ANSWER>
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//

/**
 * Process 0: copy the image directly to the output file
 * @param vector of the input BMP image as read by vector<vector<Pixel>> read_image(string filename)
 */
vector<vector<Pixel>> process_0(const vector<vector<Pixel>>& image) {
    int num_rows = image.size();
    int num_columns = image[0].size();

    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));

    for(int row = 0; row < num_rows; row++) {
        for(int col = 0; col < num_columns; col++) {
            //store each pixel value at vector index
            int red_color = image[row][col].red;
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;

            //set each output pixel value to the input pixel value at vector index
            new_image[row][col].red = red_color;
            new_image[row][col].green = green_color;
            new_image[row][col].blue = blue_color;
        }
    }
    return new_image;
};

/**
 * Process 1: Vignette
 * @param vector of the input BMP image as read by vector<vector<Pixel>> read_image(string filename)
 */
vector<vector<Pixel>> process_1(const vector<vector<Pixel>>& image) {
    int num_rows = image.size();
    int num_columns = image[0].size();

    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));

    for(int row = 0; row < num_rows; row++) {
        for(int col = 0; col < num_columns; col++) {
            //find distance to center
            double distance = sqrt(pow((col - num_columns/2), 2) + pow((row - num_rows/2), 2));
            double scaling_factor = (num_columns - distance)/num_columns;

            //store each pixel value at vector index
            int red_color = image[row][col].red;
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;

            //calculate new color values
            double new_red = red_color * scaling_factor;
            double new_blue = blue_color * scaling_factor;
            double new_green = green_color * scaling_factor;

            //set each output pixel value to the input pixel value at vector index
            new_image[row][col].red = new_red;
            new_image[row][col].green = new_green;
            new_image[row][col].blue = new_blue;
        }
    }
    return new_image;
};

/**
 * Process 2: Clarendon
 * @param vector of the input BMP image as read by vector<vector<Pixel>> read_image(string filename)
 */
vector<vector<Pixel>> process_2(const vector<vector<Pixel>>& image) {
    int num_rows = image.size();
    int num_columns = image[0].size();
    double scaling_factor = 0.3;

    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));

    for(int row = 0; row < num_rows; row++) {
        for(int col = 0; col < num_columns; col++) {
            //store each pixel value at vector index
            int red_color = image[row][col].red;
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;

            //calculate average of rgb values
            double average_value = (red_color + blue_color + green_color)/3;

            //if the pixel is light, make it lighter
            double new_red;
            double new_blue;
            double new_green;

            if(average_value >= 170) {
                new_red = int(255 - (255 - red_color)*scaling_factor);
                new_blue =  int(255 - (255 - blue_color)*scaling_factor);
                new_green = int(255 - (255 - green_color)*scaling_factor);
            }
            else if(average_value < 90) {
                new_red = red_color * scaling_factor;
                new_blue =  blue_color * scaling_factor;
                new_green = green_color * scaling_factor;
            }
            else{
                new_red = red_color;
                new_blue =  blue_color;
                new_green = green_color;
            }

            //set each output pixel value to the input pixel value at vector index
            new_image[row][col].red = new_red;
            new_image[row][col].green = new_green;
            new_image[row][col].blue = new_blue;
        }
    }
    return new_image;
};

/**
 * Process 3: Grayscale
 * @param vector of the input BMP image as read by vector<vector<Pixel>> read_image(string filename)
 */
vector<vector<Pixel>> process_3(const vector<vector<Pixel>>& image) {
    int num_rows = image.size();
    int num_columns = image[0].size();

    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));

    for(int row = 0; row < num_rows; row++) {
        for(int col = 0; col < num_columns; col++) {
            //store each pixel value at vector index
            int red_color = image[row][col].red;
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;

            //average the pixel values
            double gray_value = (red_color + blue_color + green_color) / 3;

            //set each output pixel value to the input pixel value at vector index
            new_image[row][col].red = gray_value;
            new_image[row][col].green = gray_value;
            new_image[row][col].blue = gray_value;
        }
    }
    return new_image;
};

/**
 * Process 4: Rotate 90 degrees
 * @param vector of the input BMP image as read by vector<vector<Pixel>> read_image(string filename)
 */
vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image) {
    int num_rows = image.size();
    int num_columns = image[0].size();

    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));

    for(int row = 0; row < num_rows; row++) {
        for(int col = 0; col < num_columns; col++) {
            //store each pixel value at vector index
            int red_color = image[row][col].red;
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;

            //adjust new pixel location
            int new_row = (num_rows - 1) - row;

            //set each output pixel value to the input pixel value at vector index
            new_image[col][new_row].red = red_color;
            new_image[col][new_row].green = green_color;
            new_image[col][new_row].blue = blue_color;
        }
    }
    return new_image;
};

/**
 * Process 5: Rotate multiple 90 degrees
 * @param vector of the input BMP image as read by vector<vector<Pixel>> read_image(string filename)
 */
vector<vector<Pixel>> process_5(const vector<vector<Pixel>>& image) {
    int num_rows = image.size();
    int num_columns = image[0].size();

    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));

    for(int row = 0; row < num_rows; row++) {
        for(int col = 0; col < num_columns; col++) {
            //store each pixel value at vector index
            int red_color = image[row][col].red;
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;

            //set each output pixel value to the input pixel value at vector index
            new_image[row][col].red = red_color;
            new_image[row][col].green = green_color;
            new_image[row][col].blue = blue_color;
        }
    }
    return new_image;
};

/**
 * Process 6: Enlarge
 * @param vector of the input BMP image as read by vector<vector<Pixel>> read_image(string filename)
 */
vector<vector<Pixel>> process_6(const vector<vector<Pixel>>& image, int x_scale, int y_scale) {
    int num_rows = image.size();
    int num_columns = image[0].size();

    //scale the size for the new image
    int new_num_rows = num_rows * y_scale;
    int new_num_cols = num_columns * x_scale;

    //create the new image vector with the scaled size
    vector<vector<Pixel>> new_image(new_num_rows, vector<Pixel>(new_num_cols));

    for(int row = 0; row < (num_rows * x_scale); row++) {
        for(int col = 0; col < (num_columns * y_scale); col++) {
            //store each pixel value at vector index
            int red_color = image[row / y_scale][col / x_scale].red;
            int blue_color = image[row / y_scale][col / x_scale].blue;
            int green_color = image[row / y_scale][col / x_scale].green;

            //set each output pixel value to the input pixel value at vector index
            new_image[row][col].red = red_color;
            new_image[row][col].green = green_color;
            new_image[row][col].blue = blue_color;
        }
    }
    return new_image;
};

/**
 * Process 7: High contrast, black and white
 * @param vector of the input BMP image as read by vector<vector<Pixel>> read_image(string filename)
 */
vector<vector<Pixel>> process_7(const vector<vector<Pixel>>& image) {
    int num_rows = image.size();
    int num_columns = image[0].size();

    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));

    for(int row = 0; row < num_rows; row++) {
        for(int col = 0; col < num_columns; col++) {
            //store each pixel value at vector index
            int red_color = image[row][col].red;
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;

            //average to get gray value
            double gray_color = (red_color + blue_color + green_color) / 3;

            int new_red;
            int new_blue;
            int new_green;

            //set black or white based on threshold
            if(gray_color >= (255 / 2)) {
                new_red = 255;
                new_blue = 255;
                new_green = 255;
            }
            else {
                new_red = 0;
                new_blue = 0;
                new_green = 0;  
            }

            //set each output pixel value to the input pixel value at vector index
            new_image[row][col].red = new_red;
            new_image[row][col].blue = new_blue;
            new_image[row][col].green = new_green;
        }
    }
    return new_image;
};

/**
 * Process 8: Lighten by scaling factor
 * @param vector of the input BMP image as read by vector<vector<Pixel>> read_image(string filename)
 */
vector<vector<Pixel>> process_8(const vector<vector<Pixel>>& image) {
    int num_rows = image.size();
    int num_columns = image[0].size();

    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));

    for(int row = 0; row < num_rows; row++) {
        for(int col = 0; col < num_columns; col++) {
            //store each pixel value at vector index
            int red_color = image[row][col].red;
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;

            double scaling_factor = .8;
            int new_red = int(255 - (255 - red_color) * scaling_factor);
            int new_blue = int(255 - (255 - blue_color) * scaling_factor);
            int new_green = int(255 - (255 - green_color) * scaling_factor);

            //set each output pixel value to the input pixel value at vector index
            new_image[row][col].red = new_red;
            new_image[row][col].green = new_green;
            new_image[row][col].blue = new_blue;
        }
    }
    return new_image;
};

/**
 * Process 9: Darken by scaling factor
 * @param vector of the input BMP image as read by vector<vector<Pixel>> read_image(string filename)
 */
vector<vector<Pixel>> process_9(const vector<vector<Pixel>>& image) {
    int num_rows = image.size();
    int num_columns = image[0].size();

    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));

    for(int row = 0; row < num_rows; row++) {
        for(int col = 0; col < num_columns; col++) {
            //store each pixel value at vector index
            int red_color = image[row][col].red;
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;

            double scaling_factor = .8;
            int new_red = red_color * scaling_factor;
            int new_blue = blue_color * scaling_factor;
            int new_green = green_color * scaling_factor;

            //set each output pixel value to the input pixel value at vector index
            new_image[row][col].red = new_red;
            new_image[row][col].green = new_green;
            new_image[row][col].blue = new_blue;
        }
    }
    return new_image;
};

/**
 * Process 10: Convert to black, white, red, blue, and green
 * @param vector of the input BMP image as read by vector<vector<Pixel>> read_image(string filename)
 */
vector<vector<Pixel>> process_10(const vector<vector<Pixel>>& image) {
    int num_rows = image.size();
    int num_columns = image[0].size();

    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));

    for(int row = 0; row < num_rows; row++) {
        for(int col = 0; col < num_columns; col++) {
            //store each pixel value at vector index
            int red_color = image[row][col].red;
            int blue_color = image[row][col].blue;
            int green_color = image[row][col].green;

            int max_rb = max(red_color, blue_color);
            int max_color = max(max_rb, green_color);

            int new_red;
            int new_blue;
            int new_green;

            if((red_color + blue_color + green_color) >= 550) {
                new_red = 255;
                new_blue = 255;
                new_green = 255;
            }
            else if((red_color + blue_color + green_color) <= 150) {
                new_red = 0;
                new_blue = 0;
                new_green = 0;
            }
            else if(max_color == red_color) {
                new_red = 255;
                new_blue = 0;
                new_green = 0;
            }
            else if(max_color == green_color) {
                new_red = 0;
                new_blue = 0;
                new_green = 255;
            }
            else {
                new_red = 0;
                new_blue = 255;
                new_green = 0;
            }

            //set each output pixel value to the input pixel value at vector index
            new_image[row][col].red = new_red;
            new_image[row][col].green = new_green;
            new_image[row][col].blue = new_blue;
        }
    }
    return new_image;
};


int main() {
    //run the CLI for the image processing app

    //startup
    cout << "CSPB 1300 Image Processing Application" << endl;
    cout << "Enter input BMP filename: " << endl;
    
    //store input file name
    string input_file;
    cin >> input_file;

    //define to control loop, checking for 'Q'
    string menu_selection;

    do{
        //display CLI menu
        cout << "IMAGE PROCESSING MENU" << endl;
        cout << "0) Change image (current: " << input_file << ")" << endl;
        cout << "1) Vignette" << endl;
        cout << "2) Clarendon" << endl;
        cout << "3) Grayscale" << endl;
        cout << "4) Rotate 90 degrees" << endl;
        cout << "5) Rotate multiple 90 degrees" << endl;
        cout << "6) Enlarge" << endl;
        cout << "7) High contrast" << endl;
        cout << "8) Lighten" << endl;
        cout << "9) Darken" << endl;
        cout << "10) Black, white, red, green, blue" << endl;

        cout << endl;
        cout << "Enter menu selection (Q to quit): ";

        //store menu selection
        cin >> menu_selection;

        //quit interrupt
        if(menu_selection == "Q") {
            break;
        }

        //receive menu_selection
        //store output file name
        cout << endl;
        cout << menu_selection << " selected" << endl;

        //Check for input 0 and handle changing the input file name
        if(menu_selection == "0") {
            cout << "Enter new input BMP filename: ";
            cin >> input_file;
            cout << "Successfully changed input image!" << endl;
        }
        else {
            cout << "Enter output BMP filename: ";
            string output_file;
            cin >> output_file;
    
            //action
            //call read_image()
            vector<vector<Pixel>> input_bmp = read_image(input_file);
    
            //call processing function that was selected
            vector<vector<Pixel>> processed_image;
            string selection_name;

            if(menu_selection == "1") {
                processed_image = process_1(input_bmp);
                selection_name = "Vignette";
            }
            else if(menu_selection == "2") {
                processed_image = process_2(input_bmp);
                selection_name = "Clarendon";
            }
            else if(menu_selection == "3") {
                processed_image = process_3(input_bmp);
                selection_name = "Grayscale";
            }
            else if(menu_selection == "4") {
                processed_image = process_4(input_bmp);
                selection_name = "Rotate 90 degrees";
            }
            else if(menu_selection == "5") {
                processed_image = process_5(input_bmp);
                selection_name = "Rotate multiple 90 degrees";
            }
            else if(menu_selection == "6") {
                int x_scale;
                int y_scale;
                cout << "Enter x scale: ";
                cin >> x_scale;
                cout << endl;
                cout << "Enter y scale: ";
                cin >> y_scale;
                processed_image = process_6(input_bmp, x_scale, y_scale);
                selection_name = "Enlarge";
            }
            else if(menu_selection == "7") {
                processed_image = process_7(input_bmp);
                selection_name = "High contrast";
            }
            else if(menu_selection == "8") {
                processed_image = process_8(input_bmp);
                selection_name = "Lighten";
            }
            else if(menu_selection == "9") {
                processed_image = process_9(input_bmp);
                selection_name = "Darken";
            }
            else if(menu_selection == "10") {
                processed_image = process_10(input_bmp);
                selection_name = "Black, white, red, green, blue";
            }
    
            //store the bool output of write_image()
            bool success = write_image(output_file, processed_image);
    
            //result
            //check for successful return from write_image()
            if(success) {
                cout << "Successfully applied " << selection_name << "!" << endl;
            }
            else {
                cout << "Failed" << endl;
            }
        }

    }
    while(menu_selection != "Q");

    return 0;
}