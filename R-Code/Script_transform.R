#libraries in use
library(RSQLite)
library(DBI)
library(sp)
library(rgdal)

setwd("D:/Users/Georg/Documents/R_workspace/Vibration/")

#Read Data and export KML in WGS84
db <- dbConnect(SQLite(), dbname = 'data.db');
data_db <- dbReadTable(db,"vibration")
coordinates(data_db)<- c("lon", "lat")
proj4string(data_db)=CRS("+proj=longlat +datum=WGS84 +ellps=WGS84")
writeOGR(data_db, dsn="data_db_full.kml", layer= "data_wgs84", driver="KML", dataset_options=c("NameField=name"))

#path and vibration segmentation
path_data <- data_db[which(data_db$vibration_type==7), ]
vibration_data <- data_db[which(data_db$vibration_type!=7), ]
b_box <- bbox(path_data)

#remove duplicates
path_data_filter <-remove.duplicates(path_data)
#remove.duplicates(vibration_data)

#create path polygon
x <- nrow(path_data_filter)
pathes <- list()
for(i in 1:x){
  coords <- matrix(cbind(c(path_data_filter$lon[i],path_data_filter$lon[i+1]),c(path_data_filter$lat[i],path_data_filter$lat[i+1])), nrow =2)
  temp_line <-Line(coords)
  pathes <- c(pathes, temp_line)
}
all_path = Lines(pathes,ID="a")
all_path = SpatialLines(list(all_path))
#all_path = SpatialLines(pathes,ID="a")

#Snap Vibration to path
#snapPointsToLines(points, lines, maxDist = NA, withAttrs = TRUE, idField=NA)