import skimage
import numpy as np
import sys

#Reading the image
img = skimage.data.chelsea()
skimage.io.imshow(img)
skimage.io.show()

rgb_image = skimage.color.rgb2gray(img)
print("rgb image dim: ", rgb_image.shape)
skimage.io.imshow(rgb_image)
skimage.io.show()

l1_filter = np.zeros((2, 3, 3))
l1_filter[0, :, :] = np.array([
    [[-1, 0, 1],
    [-1, 0, 1],
    [-1, 0, 1]]

])
l1_filter[1, :, :] = np.array([
    [[-1, -1, -1],
    [0, 0, 0],
    [1, 1, 1]]

])


def pointwise_conv(img, filter):
    feature_map = np.zeros((img.shape[0] - filter.shape[0] + 1, img.shape[1] - filter.shape[1] + 1))
    for r in range(0, img.shape[0] - filter.shape[0] + 1):
        for c in range(0, img.shape[1] - filter.shape[1] + 1):
            conv_area = img[r:r+filter.shape[0], c:c+filter.shape[1]]
            feature_map[r, c] = img[conv_area]*filter

    return feature_map 



def convolution(img, filter_bank):
    if len(img.shape) > 2 and len(filter_bank.shape) > 3: 
        # Check that the depth dimension (number of channels) is the same in the image and the convolution filter
        if img.shape[-1] != filter_bank.shape[-1]:
            print("Number of channels in images don't match the depth of the convolution filters")
            sys.exit()

        # Filter bank should be a sqaure matrix
        if filter_bank.shape[1] != filter_bank.shape[2]:
            print("Filter bank should be a square matrix")
            sys.exit()

        # Filter dimensions should be odd numbers
        if filter_bank.shape[1]%2:
            print("Filter dimensions cannot be even numbers")
            sys.exit()

        convolved_feature_map = np.array((img.shape[0] - filter_bank.shape[1] + 1,
                                          img.shape[1] - filter_bank.shape[2] + 1,
                                        filter_bank[0]))
        
        for filter_num in range(filter_bank.shape[0]):
            print("Filter: ", filter_num)
            curr_filter = filter_bank[filter_num, :]
            feature_map = pointwise_conv(img[:, :, 0], curr_filter[:, :, 0])
            # If image has a color channel
            if len(img.shape) > 2:
                for channel in range(1, img.shape[-1]):
                    feature_map += pointwise_conv(img[:, :, channel], curr_filter[:, :, channel])

                
            convolved_feature_map[:, :, filter_num] = feature_map
                





