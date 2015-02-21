#libraries in use
library(RSQLite)
library(DBI)
library(sp)
library(maptools)
library(rgdal)
library(data.table)

setwd("D:/Users/Georg/Documents/R_workspace/Vibration/")

#Read Data and export KML in WGS84
db <- dbConnect(SQLite(), dbname = 'data.db');
data_db <- dbReadTable(db,"vibration")
coordinates(data_db)<- c("lon", "lat")
proj4string(data_db)=CRS("+proj=longlat +datum=WGS84 +ellps=WGS84")
writeOGR(data_db, dsn="data_db_full.kml", layer= "data_wgs84", driver="KML", dataset_options=c("NameField=name"))

#Read bikelanes
bikelanes <- readOGR(dsn="bikelanes.geojson", layer="OGRGeoJSON", p4s="+proj=longlat +datum=WGS84 +ellps=WGS84")
proj4string(bikelanes)=CRS("+proj=longlat +datum=WGS84 +ellps=WGS84")

#Snap vibration events to bikelanes, scrap rest
data_db_snap <-snapPointsToLines(data_db, bikelanes, maxDist = 0.0001, withAttrs = TRUE, idField=NA)

#debug
writeOGR(bikelanes, dsn="bikelanes.kml", layer= "data_wgs84", driver="KML", dataset_options=c("NameField=name"))
writeOGR(data_db_snap, dsn="data_db_snap.kml", layer= "data_wgs84", driver="KML", dataset_options=c("NameField=name"))

#path and vibration segmentation
path_data <- data_db_snap[which(data_db_snap$vibration_type==7), ]
vibration_data <- data_db_snap[which(data_db_snap$vibration_type!=7), ]
b_box <- bbox(path_data)

#remove duplicates
path_data_filter <-remove.duplicates(path_data)
#remove.duplicates(vibration_data)

#create path polygon and export
x <- nrow(path_data_filter)
pathes <- list()
se <- list()
for(i in 1:x){
  # no Overshoot
  if(i!=x){
  nam <- paste("path_sg ", i, sep = "")
  coords <- matrix(cbind(c(path_data_filter$X[i],path_data_filter$X[i+1]),c(path_data_filter$Y[i],path_data_filter$Y[i+1])), nrow =2)
  temp_line <-Line(coords)
  pathes <- c(pathes,temp_line)
  all_path <- Lines(pathes,ID="path_sq ")
  se <- c(se, paste(nam))
  }
}
se <-data.frame(se, row.names = c("path_sq "))
all_path <- SpatialLines(list(all_path), proj4string = CRS("+proj=longlat +datum=WGS84 +ellps=WGS84"))
path = SpatialLinesDataFrame(all_path, se, match.ID = TRUE)
writeOGR(path, dsn="path.kml", layer= "path", driver="KML")


#Create martix vibration Events
vibration_st <- matrix(c(vibration_data$X,vibration_data$Y,vibration_data$runtime,(sqrt(vibration_data$imu_x^2+vibration_data$imu_y^2+vibration_data$imu_z^2)),vibration_data$vibration_type),ncol=5)

#Test strong Sensor
sens_test <- vibration_st[which(vibration_st[,5]>=4), ]
length(sens_test)

#Filter: over 2G, under 5G
vibration_st <- vibration_st[which(vibration_st[,4]>=4), ]
vibration_st <- vibration_st[which(vibration_st[,4]<=25), ]

#Sum Data from identical coordinates
agg_vib <- data.frame(vibration_st)
agg_vib <- as.data.table(agg_vib)
agg_vib <- agg_vib[, list(sum(X4)), by=list(X1,X2)]
 
#Create Vibration Event polygon
x <- nrow(agg_vib)
pathes_vib <- list()
se_vib <- vector()
all_vib <- list()
amoun <- vector()
for(i in 1:x){
  #no overshoot
  if(i!=x){
    #do not connect points too far away
    if((abs(agg_vib$X1[i]-agg_vib$X1[i+1]) <= 0.005) && (abs(agg_vib$X2[i]-agg_vib$X2[i+1]) <= 0.005)){
      nam <- paste("path_sg ", i, sep = "")
      coords <- cbind(c(agg_vib$X1[i],agg_vib$X1[i+1]),c(agg_vib$X2[i],agg_vib$X2[i+1]))
      temp_line <- Line(coords)
      temp_lines <- Lines(temp_line, ID =(paste("path_sg ", i, sep = "")))
      all_vib <- c(all_vib,temp_lines)
      se_vib <- c(se_vib, paste(nam))
      amoun <- c(amoun, agg_vib$V1[i])
    }
  }
}
amoun <-data.frame(se_vib, amoun)
all_vib <- SpatialLines(all_vib)
proj4string(all_vib)=CRS("+proj=longlat +datum=WGS84 +ellps=WGS84")
path_vib = SpatialLinesDataFrame(all_vib, amoun, match.ID = FALSE)
writeOGR(path_vib, dsn="path_vib", layer= "path_vibration", driver="GeoJSON", dataset_options=c("NameField=name"))
