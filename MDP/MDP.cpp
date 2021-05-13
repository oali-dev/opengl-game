/* This is an example implementation of the midpoint displacement algorithm in 2 dimensions. */
/* Written by Omar Ali */

#include <iostream>
#include <SFML/Graphics.hpp>
#include <vector>
#include <stdlib.h>
#include <iomanip>
using namespace std;

void gen_height_map(std::vector< std::vector<float> >& height_map, const unsigned int map_size = 65, 
				    double base = 0.0f, double offset_range = 50.0f, double roughness = 0.5f) 
{
	height_map.resize(map_size);
	for(std::vector< std::vector<float> >::iterator it = height_map.begin();
		it != height_map.end();
		it++)
	{
		(*it).resize(map_size);
	}

	bool isSide_x;
	bool isSide_y;
	
    /* Initializing the corners. */
    height_map[0][0] = height_map[0][map_size - 1] = height_map[map_size - 1][0] = height_map[map_size - 1][map_size - 1] = base;

    /* Iterate through the map */
	for( int sideLength = (map_size - 1) / 2; sideLength > 0; sideLength /= 2, offset_range *= roughness ) {

        isSide_y = false; // Initially false, since we are at a corner.

        for( int i = 0; i < map_size; i += sideLength  ) {

            isSide_x = false; // Initially false, since we are at a corner.

            for( int j = 0; j < map_size; j += sideLength ) {

                if( isSide_x && isSide_y ) { // Are we at a center point?
                    height_map[j][i] = ((height_map[j + sideLength][i + sideLength] +
                                         height_map[j + sideLength][i - sideLength] +
                                         height_map[j - sideLength][i + sideLength] +
                                         height_map[j - sideLength][i - sideLength]) / 4) +
                                         ( 2*(rand() % ( 1 /* So we don't get a value modulo 0 */ + (int)offset_range)) - offset_range);
                }

                else if( isSide_x ) {
                    height_map[j][i] = ((height_map[j + sideLength][i] +
                                        height_map[j - sideLength][i]) / 2) +
                                        ( 2*(rand() % (1 + (int)offset_range)) - offset_range);
                }

                else if( isSide_y ) { 
                    height_map[j][i] = ((height_map[j][i + sideLength] +
                                         height_map[j][i - sideLength]) / 2) +
                                         ( 2*(rand() % (1 + (int)offset_range)) - offset_range);
                }

                isSide_x = !isSide_x;
            }

            isSide_y = !isSide_y;
        }
    }

	sf::Image image;
	image.create(map_size, map_size, sf::Color::White);
	for(int i = 0; i < map_size; i++)
	{
		for(int j = 0; j < map_size; j++)
		{
			float height = height_map[i][j];
			image.setPixel(i, j, sf::Color(height, height, height));
		}
	}

	image.saveToFile("heightmap.png");
}

int main(int argc, char** argv)
{
	int size = 65;
	double base = 0.0f;
	double range = 128.0f;
	double roughness = 0.6f;
	
	do {
		cout << "Enter the resolution (must be of the form 2^n + 1 where n is a positive integer)" << endl;
		cin >> size;
	} while( !( ( ( (size - 1) & (size - 2) ) == 0) && (size - 1 != 0) ) );	// Keep asking for input until we get a number of the form 2^n + 1


	do {
		cout << "Enter the roughness constant between 0 and 1 exclusive" << endl;
		cin >> roughness;
	} while( !(roughness > 0.0 && roughness < 1.0) ); // Keep asking for input until we get a number in the range (0, 1)

	std::vector< std::vector<float> > heightmap;
	gen_height_map(heightmap, size, base, range, roughness);

	cout << "Height map created in MDP/heightmap.jpg" << endl;
	system("pause");
	return 0;
}
