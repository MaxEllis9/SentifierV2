import tensorflow as tf
import sys

# change this as you see fit
image_path = sys.argv[1]

# Read in the image_data
image_data = tf.compat.v1.gfile.FastGFile(image_path, 'rb').read()

# Loads label file, strips off carriage return
label_lines = [line.rstrip() for line 
                   in tf.compat.v1.gfile.GFile("/Users/max/plugInDev/distortionPlugInV2/classifier/retrained_labels.txt")]

# Unpersists graph from file
with tf.compat.v1.gfile.FastGFile("/Users/max/plugInDev/distortionPlugInV2/classifier/retrained_graph.pb", 'rb') as f:
    graph_def = tf.compat.v1.GraphDef()
    graph_def.ParseFromString(f.read())
    _ = tf.import_graph_def(graph_def, name='')

with tf.compat.v1.Session() as sess:
    # Feed the image_data as input to the graph and get first prediction
    softmax_tensor = sess.graph.get_tensor_by_name('final_result:0')
    
    predictions = sess.run(softmax_tensor, \
             {'DecodeJpeg/contents:0': image_data})
    
    # Sort to show labels of first prediction in order of confidence
    top_k = predictions[0].argsort()[-len(predictions[0]):][::-1]
    
    outputFile = open('/Users/max/plugInDev/distortionPlugInV2/tensor-guess-master/output.txt', 'w')
    humanOutputFile = open('/Users/max/plugInDev/distortionPlugInV2/tensor-guess-master/humanOutput.txt', 'w')
    
    for node_id in top_k:
        human_string = label_lines[node_id]
        score = predictions[0][node_id]
        print('%s (score = %.5f)' % (human_string, score))
        outputFile.write('%s,%.5f' % (human_string, score))
        outputFile.write("\n")
        humanOutputFile.write('%s score = %.2f%%' % (human_string, score * 100))
        humanOutputFile.write("\n")

outputFile.close()
humanOutputFile.close()

