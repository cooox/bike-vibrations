#libraries in use
library(RSQLite)
library(DBI)
library(sp)
library(maptools)
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
all_path = Lines(pathes,ID="path")
all_path = SpatialLines(list(all_path), proj4string = CRS("+proj=longlat +datum=WGS84 +ellps=WGS84"))
path = SpatialLinesDataFrame(all_path, data.frame(Z = c("Road"), row.names = c("lines")), match.ID = FALSE)
#all_path = SpatialLines(pathes,ID="a")

#Create martix vibrationEvents
vibration_st <- matrix(c(vibration_data$lon,vibration_data$lat,vibration_data$runtime,(sqrt(vibration_data$imu_x^2+vibration_data$imu_y^2+vibration_data$imu_z^2)),vibration_data$vibration_type),ncol=5)

#Test strong Sensor
sens_test <- vibration_st[which(vibration_st[,5]>=4), ]
length(sens_test)

#Filter: over 2G, under 5G
vibration_st <- vibration_st[which(vibration_st[,4]>=4), ]
vibration_st <- vibration_st[which(vibration_st[,4]<=25), ]
#Eliminate duplicate Events
x <- nrow(vibration_st)
i=1
for(i in 1:(x-1)){
vibration_st <- vibration_st[which((vibration_st[i,3]-vibration_st[(i+1),3])>=100), ]
}
#Snap Vibration to path
#snapPointsToLines(vibration_st, pathes, maxDist = NA, withAttrs = TRUE, idField=NA)