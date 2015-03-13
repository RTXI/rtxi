#! /usr/bin/env Rscript

options(warn=-1)
args = commandArgs(trailingOnly = T)

# Check if needed packages are installed. Otherwise, exit. 
if( !(require(ggplot2)&&require(scales)&&require(plyr)&&require(gridExtra)) ) {
   message("I need some R packages installed. (ggplot2, plyr, gridExtra, and scales)")
   message(prompt="Want me to install them? (y/N)")
   val <- scan("stdin", character(), n=1)
   if ((val == "y")||(val == "Y")) {
      install.packages("ggplot2", repos="http://watson.nci.nih.gov/cran_mirror/")
      install.packages("scales", repos="http://watson.nci.nih.gov/cran_mirror/")
      install.packages("plyr", repos="http://watson.nci.nih.gov/cran_mirror/")
      install.packages("gridExtra", repos="http://watson.nci.nih.gov/cran_mirror/")

      require(ggplot2)
      require(scales)
      require(plyr)
      require(gridExtra)
   } else {
      stop("Look, you need to install 'ggplot2', 'plyr', 'gridExtra', and 'scales' to do this.")
   }
}

# Set parameters passed to script from command line / bash script
os = as.character(args[1])
hostname = as.character(args[2])
rt_patch = as.character(args[3])
processor = as.character(args[4])
graphics_card = as.character(args[5])
graphics_driver = as.character(args[6])
rate = as.integer(args[7]) # kHz

# Open file and get raw data
data.raw = read.table("test_rt_histdata.txt", header=F, col.names=c("Latency", "Count"))
data.stats = data.raw
data.stats$Count = data.raw$Count - 1 # ./latency adds 1 to all fields (prevents 0s on log plots)

# Find summary stats for all points. Counts for latencies are accounted for. 
weird_mean = sum(data.stats$Count*data.stats$Latency)/(sum(data.stats$Count))
weird_sdev = sqrt( 1/sum(data.stats$Count) * sum(data.stats$Count*(data.stats$Latency - weird_mean)^2) )
data.summary = data.frame(Measure = c("Mean (μs)", "Std.Dev (μs)"), Value = c(weird_mean, weird_sdev))

# System info passed from command line
data.system = data.frame(Field = c("Operating System", "Host Name", "RT Kernel", "Processor", "Graphics Card", "Graphics Driver"), Info = c(os, hostname, rt_patch, processor, graphics_card, graphics_driver) )

# Put stats into bin_size ns bins for histogram
bin_size = 200 # bins are to make the histogram look better
data.hist = data.frame( Latency = seq( 0, max(data.stats$Latency) * 
                                       1000 + bin_size, bin_size ) ) / 1000
data.hist$Count = rep( 0, length(data.hist$Latency) )
hist_idx = 1
for ( stats_idx in 1:length(data.stats$Latency) ) {
	if ( data.stats$Latency[stats_idx] < data.hist$Latency[hist_idx] ) {
		data.hist$Count[hist_idx] = data.hist$Count[hist_idx] + data.stats$Count[stats_idx]
	} else {
		hist_idx = hist_idx + 1
		data.hist$Count[hist_idx] = data.hist$Count[hist_idx] + data.stats$Count[stats_idx]
	}
}
data.hist$Count = data.hist$Count + 1

# Create function to show probability of losing RT as a function of RT period over 1 hr...
# Plot p(stay in RT) from 1 kHz to 50 kHz
#    P(no overrun | data) ^ (Frequency * 1 hr)
min_rate = 1
max_rate = 66.7
data.prob_rt = data.frame("Frequency"=seq(min_rate, max_rate, .1))
data.prob_rt = ddply(data.prob_rt, "Frequency", function(x) {
	prob_rt =  (1 - sum( data.stats$Latency[data.stats$Latency > 1000/x$Frequency] * 
	                     data.stats$Count[data.stats$Latency > 1000/x$Frequency] ) / 
	                sum( data.stats$Latency * data.stats$Count )) ^ (x$Frequency * 1000 * 3600)
	data.frame(ProbRT = prob_rt)
})

# Start making plots of everything. 
plot.prob = ggplot(data = data.prob_rt, aes(x=Frequency, y=ProbRT)) + 
	geom_point() + 
	xlab("Frequency (kHz)") + ylab("P(Real-time over 1 hr)") + 
	scale_x_continuous(limits = c(min_rate, max_rate)) + scale_y_continuous(limits = c(0, 1.2)) 

plot.hist = ggplot(data = data.hist, aes(x=Latency, y=Count)) + 
	geom_bar(stat="identity") + 
	scale_y_log10(
		breaks = trans_breaks("log10", function(x) 10^x ),
		labels = trans_format("log10", math_format(10^.x)) ) + 
	scale_x_continuous(
		breaks = round(seq(min(data.hist$Latency), max(data.hist$Latency), by = max(data.hist$Latency)/10), 1) ) +
	xlab(expression(paste("Latency (", mu, "s)"))) 

plot.summary = qplot(1:10, 1:10, geom = "blank") + 
	theme_bw() + 
	theme(line = element_blank(), text = element_blank(), panel.grid.major = element_blank(), 
	      panel.grid.minor = element_blank(), panel.border = element_blank(), 
			panel.margin = element_blank() ) +
   annotation_custom(grob = tableGrob(data.summary, core.just="left", show.colnames=F,
	                                   row.just = "left", col.just = "left", 
												  padding.h = unit(5,"mm"), 
                                      gpar.coretext = gpar(cex=1)), 
	                  xmin=1, xmax=10, ymin=1, ymax=10)

plot.system = qplot(1:10, 1:10, geom = "blank") + 
	theme_bw() + 
	theme(line = element_blank(), text = element_blank(), panel.grid.major = element_blank(), 
	      panel.grid.minor = element_blank(), panel.border = element_blank(), 
			panel.margin = element_blank() ) +
   annotation_custom(grob = tableGrob(data.system, core.just="left", show.colnames=F,
	                                   row.just = "left", col.just = "left", 
												  padding.h = unit(5,"mm"), 
                                      gpar.coretext = gpar(cex=1)), 
	                  xmin=1, xmax=10, ymin=1, ymax=10)

# Save plots to test_rt_plot.svg. Trying to save in other formats will cause you sadness. 
svg("test_rt_plot.svg", width=12, height=9)
grid.newpage()
pushViewport(viewport(layout = grid.layout(4,6), width=1, height=1))
print(plot.system, vp = viewport(layout.pos.row = 1, layout.pos.col = 1:6))
print(plot.hist, vp = viewport(layout.pos.row = 2:4, layout.pos.col = 1:3))
print(plot.prob, vp = viewport(layout.pos.row = 2:3, layout.pos.col = 4:6))
print(plot.summary, vp = viewport(layout.pos.row = 4, layout.pos.col = 4:6))
dev.off()
