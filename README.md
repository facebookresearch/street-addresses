# Robocodes: Towards Generative Street Addresses from Satellite Imagery

This repo contains the code for creating generative street addresses from OSM input, as presented in our paper at the CVPR - EarthVision 2017. The naming procedure inputs .osm files, or geotiffs; and outputs new maps with hierarchical and linear addressing scheme. 

(New!) ISPRS IJGI paper (full system explanation): http://www.mdpi.com/2220-9964/7/3/84

CVPR EarthVision paper (best paper award): http://openaccess.thecvf.com/content_cvpr_2017_workshops/w18/papers/Demir_Robocodes_Towards_Generative_CVPR_2017_paper.pdf

Facebook Research Blog: 
https://research.fb.com/publications/robocodes-towards-generative-street-addresses-from-satellite-imagery/

SOTM US Talk: https://2017.stateofthemap.us/program/generative-street-addresses.html


Interested in satellite image understanding tasks? Take a look at our upcoming CVPR 2018 Workshop DeepGlobe at http://deepglobe.org


## Requirements

1. Install ``OpenCV => 2.4.8`` C++ bindings for road_segmentor module.

Useful links for OpenCV installation:

- Ubuntu: https://github.com/milq/milq/blob/master/scripts/bash/install-opencv.sh
- OS X: http://www.pyimagesearch.com/2015/06/15/install-opencv-3-0-and-python-2-7-on-osx/

2. Install python dependencies (``Python 2.7``).

``pip install -r requirements.txt``

## Building Robocodes
1. Clone the repo.

``$ git clone https://github.com/facebookresearch/street-addresses.git``

2. Change directory to ``${ROBOCODE}/road_segmentor`` and build. (``Cmake => 2.8``)

```
$ cmake .
$ make
```

Generated binary will be stored in ${ROBOCODE}/road_segmentor/bin

3. Run the following command to change the library paths.

``$ install_name_tool -add_rpath /<open_cv_lib_path>/ bin/RoadConnectionLabelling``

4. Check the permissions of the main python scripts ``run_end2end.py`` and ``gen_robocode.py``.


## Examples
You can check additional functionalities with ``$ ./run_end2end.py --help``. Below are some examples for easy robocode generation.

**OSM Example:** Running the script when the input is an OSM file. Creates an output osm file and additional query structure in the specified directory.

```
$ ./run_end2end.py \
 --xml ${ROBOCODE}/example/nashik.osm \
--out_dir /<output_dir>/ \
--roadSeg_bin ${ROBOCODE}/road_segmentor/bin/RoadConnectionLabelling 
```

Additional OSM files can be exported from OpenStreetMap: https://www.openstreetmap.org

**TIFF Example:** Running the script when the input is a GeoTiff file containing binary road masks. Creates an output osm file and additional query structure in the specified directory.

```
$ ./run_end2end.py \
--input_tiff ${ROBOCODE}/example/nashik.tif \
--out_dir /<output_dir>/ \
--roadSeg_bin ${ROBOCODE}/road_segmentor/bin/RoadConnectionLabelling
```

**Geocoding Example:** Generating Robocode when lat/lon is input.

```
$ ./gen_robocode.py \
-path /<input_dir>/ \
-lat 20.0226957656 \
-lon 73.7834041609 \
-city NASHIK
```

``Adress: 388A.NA104.NASHIK ``

PS: your ``lat`` and ``lon`` input should be within range of ``(minlat, maxlat)`` and ``(minlon,maxlon)`` respectively as per your input OSM or Geotiff input.

**Reverse Geocoding Example:** Generating lat/lon when Robocode is input.

```
$ ./gen_robocode.py \
-path /<input_dir>/ \
-meter 374 \
-block B \
-street NA104 \
-city NASHIK
```

``Lat, Lon: 20.0230511115, 73.7822889019``

## References
Please cite our CVPR 2017 - EarthVision Workshop paper below when using the code. 
https://research.fb.com/publications/robocodes-towards-generative-street-addresses-from-satellite-imagery/

```bibtex
@inproceedings{RobocodesCVPREV2017,
    title     = {Robocodes: Towards Generative Street Addresses from Satellite Imagery},
    author    = {Ilke Demir, Forest Hughes, Aman Raj, Kleovoulos Tsourides, Divyaa Ravichandran, Suryanarayana Murthy,
                 Kaunil Dhruv, Sanyam Garg, Jatin Malhotra, Barrett Doo, Grace Kermani, Ramesh Raskar},
    booktitle = {IEEE International Conference on Computer Vision and Pattern Recognition, EARTHVISION Workshop},
    year      = {2017} \
}
```

## License
Robocodes project is licenced under CC-by-NC, see the LICENSE file for details.
