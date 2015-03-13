#! /usr/bin/Rscript

options(warn=-1)
args = commandArgs(trailingOnly=T)

if( !(require(ggplot2)&&require(reshape2)&&require(gridExtra)) ) {
   message("I need some R packages installed. (ggplot2 and gridExtra)")
   message(prompt="Want me to install them? (y/N)")
   val <- scan("stdin", character(), n=1)
   if ((val == "y")||(val == "Y")) {
      install.packages("ggplot2", repos="http://watson.nci.nih.gov/cran_mirror/")
      install.packages("reshape2", repos="http://watson.nci.nih.gov/cran_mirror/")
      install.packages("gridExtra", repos="http://watson.nci.nih.gov/cran_mirror/")

      require(ggplot2)
      require(reshape2)
		require(gridExtra)
   } else {
      stop("Look, you need to install 'ggplot2', 'plyr', 'gridExtra', and 'scales' to do this.")
   }
}

# Headings for the plot. Edit them here. 
os = as.character(args[1])
hostname = as.character(args[2])
rt_kernel = as.character(args[3])
processor = as.character(args[4])
graphics_card = as.character(args[5])
graphics_driver = as.character(args[6])
rt_period = as.integer(args[7]) # in ns
rt_frequency = 1e6 / rt_period # in kHz
downsample = as.integer(args[8])
channel1 = as.character(args[9])
channel2 = as.character(args[10])
channel3 = as.character(args[11])
infile = as.character(args[12])
outfile = as.character(args[13])

data.raw = read.table(infile)
data.stats = data.frame(Time=seq(1, length(data.raw$V1))/(rt_frequency*1000/downsample)) # in s

# Choose whether to show time in min or hr
if (max(data.stats$Time) > 600) {
	xaxislabel = "Time (min)"
	data.stats$Time = data.stats$Time / 60 # s to min
} else if(max(data.stats$Time) > 10800) {
	xaxislabel = "Time (hr)"
	data.stats$Time = data.stats$Time / 3600 # s to hr
} else {
	xaxislabel = "Time (s)"
}

# messy, but needed to check which channel has which variable
if (channel1 == "Comp Time") {
	data.stats$CompTime = data.raw$V2
} else if (channel2 == "Comp Time") {
	data.stats$CompTime = data.raw$V3
} else if (channel3 == "Comp Time") {
	data.stats$CompTime = data.raw$V4
}
# messy, but needed
if (channel1 == "Real-time Period") {
	data.stats$Period = data.raw$V2
} else if (channel2 == "Real-time Period") {
	data.stats$Period = data.raw$V3
} else if (channel3 == "Real-time Period") {
	data.stats$Period = data.raw$V4
}
# messy, but needed
if (channel1 == "RT Jitter") {
	data.stats$Jitter = data.raw$V2
} else if (channel2 == "RT Jitter") {
	data.stats$Jitter = data.raw$V3
} else if (channel3 == "RT Jitter") {
	data.stats$Jitter = data.raw$V4
}

head(data.stats)

# Scale the data.
data.stats$CompTime  = data.stats$CompTime / 1000 # to us
data.stats$Period = data.stats$Period / 1000 # to us
data.stats$Jitter = data.stats$Jitter / 1000 # to us

# System info passed from command line
data.system = data.frame(Field = c("Operating System", "Hostname", "RT Kernel", "Processor", "Graphics Card", "Graphics Driver", "RT Frequency (kHz)", "Downsampling"), Info = c(os, hostname, rt_kernel, processor, graphics_card, graphics_driver, rt_frequency, downsample) )

data.m = melt(data.stats, id.vars='Time')

plot.system = qplot(1:2, 1:2, geom = "blank") +
   theme_bw() +
   theme(line = element_blank(), text = element_blank(), panel.grid.major = element_blank(),
         panel.grid.minor = element_blank(), panel.border = element_blank(),
         panel.margin = element_blank() ) +
   annotation_custom(grob = tableGrob(data.system, core.just="left", show.colnames=F,
                                      row.just = "left", col.just = "left",
                                      padding.h = unit(5,"mm"),
                                      gpar.coretext = gpar(cex=1)),
                     xmin=1, xmax=2, ymin=1, ymax=2)

plot.performance = ggplot(data.m, aes(x=Time, y=value, colour=variable)) +
	geom_point(shape=16, alpha=.2, cex=1) + 
	facet_wrap( ~ variable, scales="free", ncol=1) + 
	labs(x=xaxislabel, y=expression(paste("Time (", mu, "s)"))) + 
	theme(axis.text=element_text(size=16), axis.title=element_text(size=16)) +
	guides(colour=FALSE) #+

# Still not happy with how the plotting has turned out. Using png for now. -Ansel

#ggsave(filename=outfile, plot=perfplot)
png(outfile, width=700, height=900) #, width=800, height=1100)
#svg(outfile, width=5, height=8)
grid.newpage()
pushViewport(viewport(layout = grid.layout(4,1), width=1, height=1))
print(plot.system, vp = viewport(layout.pos.row = 1, layout.pos.col = 1))
print(plot.performance, vp = viewport(layout.pos.row = 2:4, layout.pos.col = 1))
dev.off()

#newthing = arrangeGrob(plot.system, perfplot, nrow=2, heights=c(.25, .75) )
#ggsave(file=outfile, newthing)
