#include "avgpool_edge.h"

AvgPoolEdge::AvgPoolEdge(const config::Edge& edge_config) :
  Edge(edge_config),
  kernel_size_(edge_config.kernel_size()),
  stride_(edge_config.stride()),
  padding_(edge_config.padding()){}

void AvgPoolEdge::SetTiedTo(Edge* e) {
  Edge::SetTiedTo(e);
  AvgPoolEdge* ee = dynamic_cast<AvgPoolEdge*> (e);
  kernel_size_ = ee->GetKernelSize();
  stride_ = ee->GetStride();
  padding_ = ee->GetPadding();
}

void AvgPoolEdge::SetImageSize(int image_size_y, int image_size_x) {
  Edge::SetImageSize(image_size_y, image_size_x);
  if (kernel_size_ <= 0) kernel_size_ = image_size_x;
  num_modules_y_ = (image_size_y + 2 * padding_ - kernel_size_) / stride_ + 1;
  num_modules_x_ = (image_size_x + 2 * padding_ - kernel_size_) / stride_ + 1;
}

string AvgPoolEdge::GetDescription() {
  stringstream ss;
  ss << name_ << " "
     << " AvgPool Kernel: " << kernel_size_ << "-" << kernel_size_ << "-"
     << num_input_channels_ << " : " << num_output_channels_
     << " Layer: " << image_size_y_ << "-" << image_size_x_ << "-"
     << num_input_channels_ << " : " << num_modules_y_ << "-" << num_modules_x_
     << "-" << num_output_channels_;
  return ss.str();
}

void AvgPoolEdge::FOV(int* size, int* sep, int* pad1, int* pad2) const {
  *size = kernel_size_ + stride_ * ((*size) - 1);
  *sep = (*sep) * stride_;
  *pad1 = (*pad1) * stride_ + padding_;
  int k = (image_size_x_ + 2*padding_ - kernel_size_) / stride_;
  int effective_right_pad = k * stride_ - (image_size_x_ + padding_ - kernel_size_);
  *pad2 = (*pad2) * stride_ + effective_right_pad;
}

void AvgPoolEdge::ComputeUp(Matrix& input, Matrix& output, bool overwrite) {
  float scale_targets = overwrite ? 0 : 1;
  // TODO: change kernel rect.
  Matrix::ConvAvgPool(input, output, num_input_channels_, kernel_size_,
                      padding_, stride_, num_modules_x_, scale_targets);
}

void AvgPoolEdge::ComputeDown(Matrix& deriv_output, Matrix& input,
                              Matrix& output, Matrix& deriv_input, bool overwrite) {
  float scale_targets = overwrite ? 0 : 1;
  // TODO: change kernel rect.
  Matrix::ConvAvgPoolUndo(deriv_output, deriv_input, kernel_size_, padding_, stride_,
                          num_modules_x_, image_size_x_, scale_targets);
}
