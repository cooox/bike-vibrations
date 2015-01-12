> ##Install the following packages (if you haven't already done so):
> 
> ##maptools, rgdal
> 
> ## Load required packages
> library(maptools)
> library(rgdal)
> 
> ##Set your working directory- this should be the folder where the above files are saved.
> 
> setwd("/KMLExport")
>                 > 
> ## Plot the XY coordinates (do not close the plot window).
> 
> plot(DATA_f$V5, DATA_f$V6)
> 
> ## create a SpatialPointsDataframe object and add the appropriate CRS
> 
> coordinates(DATA_f)<- c("V5", "V6")
> 
> BNG<- CRS("+init=epsg:27700")
> 
> proj4string(DATA_f)<-BNG
> 
> ## In order for the points to be displayed in the correct place they need to be re-projected to WGS84 geographical coordinates.
> 
> p4s <- CRS("+proj=longlat +ellps=WGS84 +datum=WGS84")
> 
> data_wgs84<- spTransform(DATA_f, CRS= p4s)
> 
> ## Using the OGR KML driver we can then export the data to KML. dsn should equal the name of the exported file and the dataset_options argument allows us to specify the labels displayed by each of the points.
> 
> writeOGR(data_wgs84, dsn="data_f.kml", layer= "data_wgs84", driver="KML", dataset_options=c("NameField=name"))