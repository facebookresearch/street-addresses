# Generative Street Addresses from Satellite Imagery

This repo contains the code for creating generative street addresses from OSM input, as presented in our paper at the CVPR - EarthVision 2017. The naming procedure inputs .osm files, or geotiffs; and outputs new maps with hierarchical and linear addressing scheme. 

## News and Resources

- Generative Street Addresses is on news: [MIT Tech Review](https://www.technologyreview.com/s/612492/four-billion-people-lack-an-address-machine-learning-could-change-that/), [Gizmodo](https://gizmodo.com/facebook-and-mit-researchers-want-to-use-ai-to-create-a-1830753801), [Engadget](https://www.engadget.com/2018/11/30/facebook-mit-assign-addresses-with-ai/), [Liberation](https://www.liberation.fr/sciences/2018/12/02/hubble-est-repare-et-le-berceau-de-l-humanite-s-est-etendu_1694747), [Slashdot](https://tech.slashdot.org/story/18/11/30/214229/researchers-are-proposing-a-new-way-to-generate-street-addresses-by-extracting-roads-from-satellite-images)

- [Generative Street Addresses from Satellite Imagery](http://www.mdpi.com/2220-9964/7/3/84), ISPRS IJGI paper with the full system explanation. 

- [Robocodes: Towards Generative Street Addresses from Satellite Imagery](https://research.fb.com/publications/robocodes-towards-generative-street-addresses-from-satellite-imagery/), best paper at CVPR 2018 EarthVision Workshop. [Facebook Research Blog article](https://research.fb.com/advancing-computer-vision-technologies-at-cvpr-2017/) mentioning the award.

- [Addressing the Invisible: Street Address Generation for Developing Countries with Deep Learning](https://arxiv.org/abs/1811.07769), NIPS 2018 ML4D Workshop paper.

- [MLConf presentation](https://www.youtube.com/watch?v=DQtV2ikrQxs) mentioning our project.

- [SOTM US presentation](https://2017.stateofthemap.us/program/generative-street-addresses.html) detailing our project. 

- MIT Media Lab partners discussing the [economic](http://mitemergingworlds.com/blog/2017/11/22/what-is-the-right-addressing-scheme-for-india) [impact](http://mitemergingworlds.com/blog/2018/2/12/economic-impact-of-discoverability-of-localities-and-addresses-in-india) of the solution. 

- Interested in other satellite image understanding tasks? Check out [DeepGlobe](http://deepglobe.org) for datasets and challenges!

- For other questions and inquiries, e-mail [idemir@purdue.edu](mailto:idemir@purdue.edu) or open an issue.


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
Please cite our [CVPR 2017 - EarthVision paper](https://research.fb.com/publications/robocodes-towards-generative-street-addresses-from-satellite-imagery/) or [IJGI paper](https://research.fb.com/publications/generative-street-addresses-from-satellite-imagery/) below when using the code. 


```bibtex
@inproceedings{demir2017robocodes,
  title={Robocodes: Towards Generative Street Addresses from Satellite Imagery},
  author={Demir, {\.I}lke and Hughes, Forest and Raj, Aman and Tsourides, Kleovoulos and Ravichandran, Divyaa and Murthy, Suryanarayana and Dhruv, Kaunil and Garg, Sanyam and Malhotra, Jatin and Doo, Barrett and Kermani, Grace and Raskar, Ramesh},
  booktitle={Computer Vision and Pattern Recognition Workshops (CVPRW), 2017 IEEE Conference on},
  pages={1486--1495},
  year={2017},
  organization={IEEE}
}
```
```bibtex
@article{demir2018IJGI,
  title={Generative Street Addresses from Satellite Imagery},
  author={Demir, {\.I}lke and Hughes, Forest and Raj, Aman and Dhruv, Kaunil and Muddala, Suryanarayana Murthy and Garg, Sanyam and Doo, Barrett and Raskar, Ramesh},
  journal={ISPRS International Journal of Geo-Information},
  volume={7},
  number={3},
  pages={84},
  year={2018},
  publisher={Multidisciplinary Digital Publishing Institute}
}
```

## License
Robocodes project is licenced under CC-by-NC, see the LICENSE file for details.
