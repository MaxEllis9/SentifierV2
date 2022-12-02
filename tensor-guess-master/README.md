[![](https://images.microbadger.com/badges/image/xblaster/tensor-guess.svg)](https://microbadger.com/images/xblaster/tensor-guess "Get your own image badge on microbadger.com")

# tensor-guess

this project is just an helper with docker images based on tutorial from https://codelabs.developers.google.com/codelabs/tensorflow-for-poets/index.html

##requirements

* docker !
* git clone this project in a directory

##usage 

You just need to make a "classifier" directory with a directory "data" inside it with all your images
For example
```
 [any_path]/my_own_classifier/
 [any_path]/my_own_classifier/data
 [any_path]/my_own_classifier/data/car
 [any_path]/my_own_classifier/data/moto
 [any_path]/my_own_classifier/data/bus
```
 and then put your image on it. 
 This "classifier" directory will have your samples but also trained classifier after execution of "train.sh". 

##Train process
 
Just type
```
 ./train.sh [any_path]/my_own_classifier
``` 
And it will do anything for you !

##Guess process

Just type for a single guess
```
 ./guess.sh [any_path]/my_own_classifier /yourfile.jpg
```

To guess an entire directory
```
./guessDir.sh [any_path]/classifier [any_path]/srcDir [any_path]/destDir
```

## Example of result
```
# ./guess.sh /synced/tensor-lib/moto-classifier/ /synced/imagesToTest/moto21.jpg
moto (score = 0.88331)
car (score = 0.11669)
```


##Remarks 

Use absolute file path for classifier and images because for the moment my script do not support relative path (volume mounting)
You can use `pwd` as a workaround
