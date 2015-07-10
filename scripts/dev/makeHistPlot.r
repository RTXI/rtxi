#! /usr/bin/env Rscript

options(warn=-1)
args = commandArgs(trailingOnly = T)

# Check if needed packages are installed. Otherwise, exit. 
if( !(require(ggplot2)&&require(scales)&&require(gridExtra)) ) {
	message("I need to have gridExtra, scales, and ggplot2 installed.")
	stop("Run the install_dependencies.sh script and try this again.")
}

# Set parameters passed to script from command line / bash script
os = as.character(args[1])
hostname = as.character(args[2])
rt_patch = as.character(args[3])
processor = as.character(args[4])
graphics_card = as.character(args[5])
graphics_driver = as.character(args[6])
rate = as.integer(args[7]) # kHz
daq = as.character(args[8])

# Open file and get raw data
data.raw = read.table("test_rt_histdata.txt", header=F, col.names=c("Latency", "Count"))
data.stats = data.raw
data.stats$Count = data.raw$Count - 1 # ./latency adds 1 to all fields (prevents 0s on log plots)

# Find summary stats for all points. Counts for latencies are accounted for. 
weird_mean = sum(data.stats$Count*data.stats$Latency)/(sum(data.stats$Count))
weird_sdev = sqrt( 1/sum(data.stats$Count) * sum(data.stats$Count*(data.stats$Latency - weird_mean)^2) )
data.summary = data.frame(Measure = c("Mean (μs)", "Std.Dev (μs)"), Value = c(weird_mean, weird_sdev))

# System info passed from command line
data.system = data.frame( Field = c("Operating System", "Host Name", "RT Kernel", "Processor", "Graphics Card", "Graphics Driver", "Test Frequency (kHz)", "DAQ" ), Info = c(os, hostname, rt_patch, processor, graphics_card, graphics_driver, rate, daq) )

# Put stats into bin_size ns bins for histogram
bin_size = 200 # bins are to make the histogram look better
if ( (max(data.stats$Latency) - min(data.stats$Latency))/bin_size * 1000 > 100 ) {
	bin_size = ( max(data.stats$Latency) - min(data.stats$Latency) ) * 1000 / 100
}
data.hist = data.frame( Latency = seq( 0, max(data.stats$Latency) * 
                                       1000 + bin_size - 1, bin_size ) ) / 1000
data.hist$Count = rep( 0, length(data.hist$Latency) )
hist_idx = 1
for ( stats_idx in 1:length(data.stats$Latency) ) {
	if ( data.stats$Latency[stats_idx] < data.hist$Latency[hist_idx] ) {
		data.hist$Count[hist_idx] = data.hist$Count[hist_idx] + data.stats$Count[stats_idx]
	} else {
		hist_idx = hist_idx + 1
		data.hist$Count[hist_idx] = data.hist$Count[hist_idx] + data.stats$Count[stats_idx]
#		print(hist_idx)
	}
}

data.hist$Count = data.hist$Count + 1

# Terminal output. For debugging. 
#print(data.summary)
#print(data.hist$Count)

# Start making plots of everything. 
plot.hist = ggplot(data = data.hist, aes(x=Latency, y=Count)) + 
	geom_bar(stat="identity") + 
	scale_y_log10(
		breaks = trans_breaks("log10", function(x) 10^x ),
		labels = trans_format("log10", math_format(10^.x)) ) + 
	scale_x_continuous(
		breaks = round(seq(min(data.hist$Latency), max(data.hist$Latency), by = max(data.hist$Latency)/10), 1) ) +
	xlab(expression(paste("Latency (", mu, "s)"))) + 
   annotation_custom(grob = tableGrob(data.summary, core.just="left", show.colnames=F,
	                                   row.just = "left", col.just = "left", 
                                      gpar.coretext = gpar(cex=1)), 
	                  xmin=.8*max(data.hist$Latency), xmax=max(data.hist$Latency), 
							ymin=-Inf, ymax=Inf)

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

# Save plots to test_rt_plot.svg. It's an svg so they can appear scaled to window size for the website. 
svg("test_rt_plot.svg", width=11, height=8.5)
grid.newpage()
pushViewport(viewport(layout = grid.layout(5,8), width=1, height=1))
print(plot.system, vp = viewport(layout.pos.row = 1:2, layout.pos.col = 1:8))
print(plot.hist, vp = viewport(layout.pos.row = 3:5, layout.pos.col = 1:8))
dev.off()
