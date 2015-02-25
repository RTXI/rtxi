#! /usr/bin/env Rscript

options(warn=-1)

data = read.table("histdata.txt", header=F, col.names=c("Latency", "Count"))

if(require(ggplot2)&&require(scales)) {
	print("yay. you have ggplot")
	
	plot = ggplot(data = data, aes(x=Latency, y=Count)) + 
		geom_bar(stat="identity") + 
		scale_y_continuous(trans=log10_trans(), 
			breaks = trans_breaks("log10", function(x) 10^x ),
			labels = trans_format("log10", math_format(10^.x)) )
#		coord_trans(y="log10")
	ggsave(filename="histplot.png", plot=plot)
} else {
	print("Okay. I'll plot the histogram for you, but just know that it'll look better with ggplot")

	png("histplot.png")
	barplot(data$Count, names.arg=data$Latency, xlab="Latency (us)", ylab="Count", log="y")
	dev.off()
}
