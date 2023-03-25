#pragma once

#include "spectrum.h"
#include "lenssystem.h"
#include <stdio.h>
#include <algorithm>

#define INTENSITY_EPS 1e-5

static inline double raytrace_dot(Eigen::Vector3d u, Eigen::Vector3d v) {
  return u(0)*v(0) + u(1)*v(1) + u(2)*v(2);
}

static inline void raytrace_cross(Eigen::Vector3d &r, const Eigen::Vector3d u, const Eigen::Vector3d v) {
  r(0) = u(1)*v(2)-u(2)*v(1);
  r(1) = u(2)*v(0)-u(0)*v(2);
  r(2) = u(0)*v(1)-u(1)*v(0);
}

static inline void raytrace_normalise(Eigen::Vector3d &v) {
  const double ilen = 1.0f/std::sqrt(raytrace_dot(v,v));
  for(int k=0;k<3;k++) v(k) *= ilen;
}

static inline void raytrace_substract(Eigen::Vector3d &u, const Eigen::Vector3d v) {
  for(int k = 0; k < 3; k++) u(k) -= v(k);
}

static inline void raytrace_multiply(Eigen::Vector3d &v, const double s) {
  for(int k = 0; k < 3; k++) v(k) *= s;
}

static inline void propagate(Eigen::Vector3d &pos, const Eigen::Vector3d dir, const double dist) {
  for(int i=0;i<3;i++) pos(i) += dir(i) * dist;
}


static inline int spherical(
  Eigen::Vector3d &pos, Eigen::Vector3d dir, double &dist, const double R, const double center, const double housing_rad, Eigen::Vector3d &normal
) {
  const Eigen::Vector3d scv(pos(0), pos(1), pos(2) - center);
  const double a = raytrace_dot(dir, dir);
  const double b = 2 * raytrace_dot(dir, scv);
  const double c = raytrace_dot(scv, scv) - R*R;
  const double discr = b*b-4*a*c;
  if(discr < 0.0f) return 4;
  int error = 0;
  double t = 0.0f;
  const double t0 = (-b-std::sqrt(discr))/(2*a);
  const double t1 = (-b+std::sqrt(discr))/(2*a);
  if(t0 < -1e-4) t = t1;
  else t = fminf(t0, t1);
  if(t < -1e-4) return 16;

  propagate(pos, dir, t);
  error |= (int)(pos(0)*pos(0) + pos(1)*pos(1) > housing_rad*housing_rad)<<4;

  normal(0) = pos(0)/R;
  normal(1) = pos(1)/R;
  normal(2) = (pos(2) - center)/R;

  dist = t;
  return error;
}

static inline double evaluate_aspherical(const Eigen::Vector2d pos, const double R, const int k, const Eigen::Vector4d correction) {
  double h = std::sqrt(pos(0)*pos(0) + pos(1)*pos(1));
  double hr = h / R;
  double h2 = h*h;
  double h4 = h2*h2;
  double h6 = h4*h2;
  double h8 = h4*h4;
  double h10 = h8*h2;
  double z = h*hr/(1+std::sqrt(std::max(0.0, 1-(1+k)*hr*hr)))+correction[0]*h4+correction[1]*h6+correction[2]*h8+correction[3]*h10;
  return z;
}

static inline double evaluate_aspherical_derivative(const Eigen::Vector2d pos, const double R, const int k, const Eigen::Vector4d correction) {
  double h = std::sqrt(pos(0)*pos(0)+pos(1)*pos(1));
  double hr = h / R;
  double h2 = h*h;
  double h3 = h2*h;
  double h5 = h3*h2;
  double h7 = h5*h2;
  double h9 = h7*h2;
  double z = 2*hr/(1+std::sqrt(std::max(0.0, 1-(1+k)*hr*hr)))+
    hr*hr*hr*(k+1)/(std::sqrt(std::max(0.0, 1-(1+k)*hr*hr))*powf(std::sqrt(std::max(0.0, 1-(1+k)*hr*hr))+1, 2))+
    4*correction[0]*h3+6*correction[1]*h5+8*correction[2]*h7+10*correction[3]*h9;
  return z;
}

static inline int aspherical(
  Eigen::Vector3d &pos, Eigen::Vector3d dir, double &dist, const double R, const double center, const int k, const Eigen::Vector4d correction, const double housing_rad, Eigen::Vector3d &normal
) {
  //first intersect sphere, then do correction iteratively
  double t = 0;
  int error = spherical(pos, dir, t, R, center, housing_rad, normal);

  double rad = R;
  Eigen::Vector4d corr(correction(0), correction(1), correction(2), correction(3));
  if(std::abs(center+R-pos(2)) > std::abs(center-R-pos(2)))
  {
    rad = -R;
    for(int i = 0; i < 4; i++) corr(i) = -corr(i); //this doesn't seem to be used? wagwan?
  }

  double position_error = 1e7;


  Eigen::Vector2d pos2f(pos(0), pos(1));
  for(int i = 0; i < 100; i++)
  {
    position_error = rad+center-pos(2)-evaluate_aspherical(pos2f, rad, k, correction);
    double tErr = position_error/dir(2);
    t += tErr;
    propagate(pos, dir, tErr);
    if(std::abs(position_error) < 1e-4) break;
  }

  double dz = evaluate_aspherical_derivative(pos2f, rad, k, correction);
  const double r = std::sqrt(pos(0)*pos(0)+pos(1)*pos(1));

  if(normal(2) < 0) dz = -dz;
  normal(0) = pos(0)/r*dz;
  normal(1) = pos(1)/r*dz;
  normal(2) /= std::abs(normal(2));

  raytrace_normalise(normal);

  dist = t;
  return error;
}


static inline int cylindrical(Eigen::Vector3d &pos, Eigen::Vector3d dir, double &dist, const double R, const double center, const double housing_rad, Eigen::Vector3d &normal, const bool cyl_y) {
  const Eigen::Vector3d scv(
    cyl_y ? pos[0] : 0.0f, 
    cyl_y ? 0.0f : pos[1],
    pos[2] - center
  );

  const double a = raytrace_dot(dir, dir);
  const double b = 2 * raytrace_dot(dir, scv);
  const double c = raytrace_dot(scv, scv) - R*R;
  
  const double discr = b*b - 4*a*c;
  if(discr < 0.0f) return 4;
  int error = 0;

  double t = 0.0f;
  if(R > 0.0f)      t = (-b-std::sqrt(discr))/(2*a);
  else if(R < 0.0f) t = (-b+std::sqrt(discr))/(2*a);

  propagate(pos, dir, t);
  error |= (int)(pos(0)*pos(0) + pos(1)*pos(1) > housing_rad*housing_rad) << 4;

  normal(0) = cyl_y ? pos(0)/R : 0.0;
  normal(1) = cyl_y ? 0.0 : pos(1)/R;
  normal(2) = (pos(2) - center)/R;

  dist = t;
  return error;
}


static inline double fresnel(const double n1, const double n2, const double cosr, const double cost) {
  if(cost <= 0.0f) return 1.0; // total inner reflection

  // fresnel for unpolarized light:
  const double Rs = (n1*cosr - n2*cost)/(n1*cosr + n2*cost);
  const double Rp = (n1*cost - n2*cosr)/(n1*cost + n2*cosr);
  return std::min(1.0, (Rs*Rs + Rp*Rp)*.5);
}

static inline double refract(const double n1, const double n2, const Eigen::Vector3d n, Eigen::Vector3d &dir) {
  if(n1 == n2) return 1;
  const double eta = n1/n2;

  const double norm = std::sqrt(raytrace_dot(dir,dir));
  const double cos1 = - raytrace_dot(n, dir)/norm;
  const double cos2_2 = 1.0-eta*eta*(1.0-cos1*cos1);

  // total (inner) reflection?
  if(cos2_2 < 0.0) return 0;
  const double cos2 = std::sqrt(cos2_2);

  for(int i=0;i<3;i++) dir(i) = dir(i)*eta/norm + (eta*cos1-cos2)*n(i);

  return 1.0-fresnel(n1, n2, cos1, cos2);
}

static inline void planeToCs(const Eigen::Vector2d inpos, const Eigen::Vector2d indir, Eigen::Vector3d &outpos, Eigen::Vector3d &outdir, const double planepos) {
  outpos(0) = inpos(0);
  outpos(1) = inpos(1);
  outpos(2) = planepos;

  outdir(0) = indir(0);
  outdir(1) = indir(1);
  outdir(2) = 1;

  raytrace_normalise(outdir);
}

static inline void csToPlane(const Eigen::Vector3d inpos, const Eigen::Vector3d indir, Eigen::Vector2d &outpos, Eigen::Vector2d &outdir, const double planepos)
{
  //intersection with plane at z = planepos
  const double t = (planepos - inpos(2)) / indir(2);

  outpos(0) = inpos(0) + t * indir(0);
  outpos(1) = inpos(1) + t * indir(1);

  outdir(0) = indir(0) / std::abs(indir(2));
  outdir(1) = indir(1) / std::abs(indir(2));
}

static inline void sphereToCs(const Eigen::Vector2d inpos, const Eigen::Vector2d indir, Eigen::Vector3d &outpos, Eigen::Vector3d &outdir, const double center, const double sphereRad)
{
  const Eigen::Vector3d normal(
    inpos(0)/sphereRad,
    inpos(1)/sphereRad,
    std::sqrt(std::max(0.0, sphereRad*sphereRad-inpos(0)*inpos(0)-inpos(1)*inpos(1)))/std::abs(sphereRad)
  );

  const Eigen::Vector3d tempDir(
    indir(0),
    indir(1),
    std::sqrt(std::max(0.0, 1.0-indir(0)*indir(0)-indir(1)*indir(1)))
  );

  Eigen::Vector3d ex(normal(2), 0, -normal(0));
  raytrace_normalise(ex);
  Eigen::Vector3d ey(0,0,0);
  raytrace_cross(ey, normal, ex);

  outdir(0) = tempDir(0) * ex(0) + tempDir(1) * ey(0) + tempDir(2) * normal(0);
  outdir(1) = tempDir(0) * ex(1) + tempDir(1) * ey(1) + tempDir(2) * normal(1);
  outdir(2) = tempDir(0) * ex(2) + tempDir(1) * ey(2) + tempDir(2) * normal(2);

  outpos(0) = inpos(0);
  outpos(1) = inpos(1);
  outpos(2) = normal(2) * sphereRad + center;
}

static inline void csToSphere(const Eigen::Vector3d inpos, const Eigen::Vector3d indir, Eigen::Vector2d &outpos, Eigen::Vector2d &outdir, const double sphereCenter, const double sphereRad)
{
  const Eigen::Vector3d normal(
    inpos(0)/sphereRad,
    inpos(1)/sphereRad,
    std::abs((inpos(2)-sphereCenter)/sphereRad)
  );

  Eigen::Vector3d tempDir(indir(0), indir(1), indir(2));
  raytrace_normalise(tempDir);

  // tangent
  Eigen::Vector3d ex(normal(2), 0, -normal(0));
  raytrace_normalise(ex);
  
  // bitangent
  Eigen::Vector3d ey(0,0,0);
  raytrace_cross(ey, normal, ex);
  
  // encode ray direction as projected position on unit disk perpendicular to the normal
  outdir(0) = raytrace_dot(tempDir, ex);
  outdir(1) = raytrace_dot(tempDir, ey);

  // outpos is unchanged, z term omitted
  outpos(0) = inpos(0);
  outpos(1) = inpos(1);
}


static inline void csToCylinder(const Eigen::Vector3d inpos, const Eigen::Vector3d indir, Eigen::Vector2d &outpos, Eigen::Vector2d &outdir, const double center, const double R, const bool cyl_y) {

  Eigen::Vector3d normal(0,0,0);
  if (cyl_y){
    normal(0) = inpos(0)/R;
    normal(2) = std::abs((inpos(2) - center)/R);
  } else {
    normal(1) = inpos(1)/R;
    normal(2) = std::abs((inpos(2) - center)/R);
  }

  Eigen::Vector3d tempDir(indir(0), indir(1), indir(2));
  raytrace_normalise(tempDir); //untested

  // tangent
  Eigen::Vector3d ex(normal(2), 0, -normal(0));
  
  // bitangent
  Eigen::Vector3d ey(0,0,0);
  raytrace_cross(ey, normal, ex);
  raytrace_normalise(ey); // not sure if this is necessary
  
  // encode ray direction as projected position on unit disk perpendicular to the normal
  outdir(0) = raytrace_dot(tempDir, ex);
  outdir(1) = raytrace_dot(tempDir, ey);

  // outpos is unchanged, z term omitted
  outpos(0) = inpos(0);
  outpos(1) = inpos(1);
}


static inline void cylinderToCs(const Eigen::Vector2d inpos, const Eigen::Vector2d indir, Eigen::Vector3d &outpos, Eigen::Vector3d &outdir, const double center, const double R, const bool cyl_y) {

  Eigen::Vector3d normal(0,0,0);
  if (cyl_y){
    normal(0) = inpos(0)/R;
    normal(2) = std::sqrt(std::max(0.0, R*R-inpos(0)*inpos(0)))/std::abs(R);
  } else {
    normal(1) = inpos(1)/R;
    normal(2) = std::sqrt(std::max(0.0, R*R-inpos(1)*inpos(1)))/std::abs(R);
  }

  const Eigen::Vector3d tempDir(
    indir(0), 
    indir(1), 
    std::sqrt(std::max(0.0, 1.0-indir(0)*indir(0)-indir(1)*indir(1)))
  );

  // tangent
  Eigen::Vector3d ex(normal(2), 0, -normal(0));
  raytrace_normalise(ex); // not sure if this is necessary
  
  // bitangent
  Eigen::Vector3d ey(0,0,0);
  raytrace_cross(ey, normal, ex);
  raytrace_normalise(ey); // not sure if this is necessary
  
  outdir(0) = tempDir(0) * ex(0) + tempDir(1) * ey(0) + tempDir(2) * normal(0);
  outdir(1) = tempDir(0) * ex(1) + tempDir(1) * ey(1) + tempDir(2) * normal(1);
  outdir(2) = tempDir(0) * ex(2) + tempDir(1) * ey(2) + tempDir(2) * normal(2);

  outpos(0) = inpos(0);
  outpos(1) = inpos(1);
  outpos(2) = normal(2) * R + center;
}


inline int intersect(const std::vector<lens_element_t> lenses, const int k,
                    Eigen::Vector3d &pos, Eigen::Vector3d dir, 
                    double &t, Eigen::Vector3d &n,
                    const double R, const double distsum,
                    const bool tracing_forward, const int draw_aspherical=1) {

  double sign = tracing_forward ? 1.0 : -1.0;

  if(stringcmp(lenses[k].geometry, "cyl-y")) return cylindrical(pos, dir, t, R, distsum + (R * sign), lenses[k].housing_radius, n, true);
  else if (stringcmp(lenses[k].geometry, "cyl-x")) return cylindrical(pos, dir, t, R, distsum + (R * sign), lenses[k].housing_radius, n, false);
  else if(stringcmp(lenses[k].geometry, "aspherical") && draw_aspherical) return aspherical(pos, dir, t, R, distsum + (R * sign), lenses[k].aspheric, lenses[k].aspheric_correction_coefficients, lenses[k].housing_radius, n);
  else return spherical(pos, dir, t, R, distsum + (R * sign), lenses[k].housing_radius, n);
}


// evalute sensor to outer pupil acounting for fresnel:
static inline int evaluate(const std::vector<lens_element_t> lenses, const int lenses_cnt, const double zoom, const Eigen::VectorXd in, Eigen::VectorXd &out, const int aspheric)
{
  int error = 0;
  double distsum = 0;
  double n1 = spectrum_eta_from_abbe_um(lenses[lenses_cnt-1].ior, lenses[lenses_cnt-1].vno, in(4));
  Eigen::Vector3d pos(0,0,0);
  Eigen::Vector3d dir(0,0,0);
  double intensity = 1.0;

  planeToCs(Eigen::Vector2d(in(0), in(1)), Eigen::Vector2d(in(2), in(3)), pos, dir, 0);

  for(int k=lenses_cnt-1;k>=0;k--)
  {
    // propagate the ray reverse to the plane of intersection optical axis/lens element:
    const double R = -lenses[k].lens_radius; // negative, evaluate() is the adjoint case
    double t = 0.0f;
    distsum += lens_get_thickness(lenses[k], zoom);

    Eigen::Vector3d normal(0, 0, 0);
    error |= intersect(lenses, k, pos, dir, t, normal, R, distsum, true);

    // index of refraction and ratio current/next:
    const double n2 = k ? spectrum_eta_from_abbe_um(lenses[k-1].ior, lenses[k-1].vno, in(4)) : 1.0; // outside the lens there is vacuum

    intensity *= refract(n1, n2, normal, dir);
    if(intensity < INTENSITY_EPS) error |= 8;
    if(error) return error;

    raytrace_normalise(dir);

    n1 = n2;
  }

  // return [x,y,dx,dy,lambda]
  Eigen::Vector2d pos_elementspace(0, 0);
  Eigen::Vector2d dir_elementspace(0, 0);
  if (stringcmp(lenses[0].geometry, "cyl-y")) csToCylinder(pos, dir, pos_elementspace, dir_elementspace, distsum-std::abs(lenses[0].lens_radius), lenses[0].lens_radius, true);
  else if (stringcmp(lenses[0].geometry, "cyl-x")) csToCylinder(pos, dir, pos_elementspace, dir_elementspace, distsum-std::abs(lenses[0].lens_radius), lenses[0].lens_radius, false);
  else csToSphere(pos, dir, pos_elementspace, dir_elementspace, distsum-std::abs(lenses[0].lens_radius), lenses[0].lens_radius);
  out(0) = pos_elementspace(0);
  out(1) = pos_elementspace(1);
  out(2) = dir_elementspace(0);
  out(3) = dir_elementspace(1);
  out(4) = intensity;
  
  return error;
}


// evalute sensor to outer pupil acounting for fresnel:
static inline int evaluate_for_pos_dir(
                  const std::vector<lens_element_t> lenses, const int lenses_cnt, 
                  const double zoom, 
                  const Eigen::VectorXd in, 
                  int aspheric, 
                  Eigen::Vector3d &pos, Eigen::Vector3d &dir, 
                  const double total_lens_length,
                  bool print_debug
                  )
{
  int error = 0;
  double n1 = spectrum_eta_from_abbe_um(lenses[lenses_cnt-1].ior, lenses[lenses_cnt-1].vno, in(4));
  double intensity = 1.0;

  planeToCs(Eigen::Vector2d(in(0), in(1)), Eigen::Vector2d(in(2), in(3)), pos, dir, -total_lens_length);

  double distsum = 0;

  for(int k=lenses_cnt-1;k>=0;k--)
  {
    // propagate the ray reverse to the plane of intersection optical axis/lens element:
    const double R = -lenses[k].lens_radius; // negative, evaluate() is the adjoint case
    double t = 0.0;
    distsum += lens_get_thickness(lenses[k], zoom);

    Eigen::Vector3d normal(0,0,0);
    error |= intersect(lenses, k, pos, dir, t, normal, R, distsum, true);
    if(error) return error;

    // index of refraction and ratio current/next:
    const double n2 = k ? spectrum_eta_from_abbe_um(lenses[k-1].ior, lenses[k-1].vno, in(4)) : 1.0; // outside the lens there is vacuum

    intensity *= refract(n1, n2, normal, dir);
    if(intensity < INTENSITY_EPS) error |= 8;
    if(error) return error;
    raytrace_normalise(dir);

    n1 = n2;

    // tmp remove
    // Eigen::Vector3d position = pos;
    // pos_list.push_back(position);
    // Eigen::VectorXd direction; direction << pos(0), pos(1), pos(2), dir(0), dir(1), dir(2);
    // dir_list.push_back(direction);

    if(print_debug) printf("[%f, %f, %f],", pos(0), pos(1), pos(2));
  }
 
  return error;
}


// evaluate scene to sensor:
static inline int evaluate_reverse(const std::vector<lens_element_t> lenses, const int lenses_cnt, const double zoom, const Eigen::VectorXd in, Eigen::VectorXd &out, const int aspheric)
{
  int error = 0;
  double n1 = 1.0;
  Eigen::Vector3d pos(0,0,0);
  Eigen::Vector3d dir(0,0,0);
  double intensity = 1.0;

  const Eigen::Vector2d inpos(in(0), in(1));
  const Eigen::Vector2d indir(in(2), in(3));
  if (stringcmp(lenses[0].geometry, "cyl-y")) cylinderToCs(inpos, indir, pos, dir, 0, lenses[0].lens_radius, true);
  else if (stringcmp(lenses[0].geometry, "cyl-x")) cylinderToCs(inpos, indir, pos, dir, 0, lenses[0].lens_radius, false);
  else sphereToCs(inpos, indir, pos, dir, 0, lenses[0].lens_radius);

  for(int i = 0; i < 2; i++) dir(i) = -dir(i);

  double distsum = 0;

  for(int k=0;k<lenses_cnt;k++)
  {
    const double R = lenses[k].lens_radius;
    double t = 0.0f;
    const double dist = lens_get_thickness(lenses[k], zoom);

    Eigen::Vector3d normal(0,0,0);
    error |= intersect(lenses, k, pos, dir, t, normal, R, distsum, true);

    // index of refraction and ratio current/next:
    const double n2 = spectrum_eta_from_abbe_um(lenses[k].ior, lenses[k].vno, in(4));
    intensity *= refract(n1, n2, normal, dir);
    if(intensity < INTENSITY_EPS) error |= 8;
    if(error) return error;

    // and renormalise:
    raytrace_normalise(dir);

    distsum += dist;
    n1 = n2;
  }

  // return [x,y,dx,dy,lambda]
  Eigen::Vector2d outpos(0,0);
  Eigen::Vector2d outdir(0,0);
  csToPlane(pos, dir, outpos, outdir, distsum);
  out(0) = outpos(0);
  out(1) = outpos(1);
  out(2) = outdir(0);
  out(3) = outdir(1);
  out(4) = intensity;

  return error;
}

static inline int evaluate_aperture(const std::vector<lens_element_t> lenses, const int lenses_cnt, const double zoom, const Eigen::VectorXd in, Eigen::VectorXd &out, const int aspheric)
{
  int error = 0;
  double n1 = spectrum_eta_from_abbe_um(lenses[lenses_cnt-1].ior, lenses[lenses_cnt-1].vno, in(4));
  Eigen::Vector3d pos(0,0,0);
  Eigen::Vector3d dir(0,0,0);
  double intensity = 1.0;

  planeToCs(Eigen::Vector2d(in(0), in(1)), Eigen::Vector2d(in(2), in(3)), pos, dir, 0);

  double distsum = 0;

  for(int k=lenses_cnt-1;k>=0;k--)
  {
    // propagate the ray reverse to the plane of intersection optical axis/lens element:
    const double R = -lenses[k].lens_radius; // negative, evaluate() is the adjoint case
    double t = 0.0;
    distsum += lens_get_thickness(lenses[k], zoom);

    // stop after moving to aperture.
    if(stringcmp(lenses[k].material, "iris")) break;

    Eigen::Vector3d normal(0,0,0);
    error |= intersect(lenses, k, pos, dir, t, normal, R, distsum, true);

    // index of refraction and ratio current/next:
    const double n2 = k ? spectrum_eta_from_abbe_um(lenses[k-1].ior, lenses[k-1].vno, in(4)) : 1.0; // outside the lens there is vacuum
    intensity *= refract(n1, n2, normal, dir);
    if(intensity < INTENSITY_EPS) error |= 8;
    if(error) return error;

    // mark this ray as theoretically dead:
    //if(dir[2] <= 0.0f) return error |= 2;
    // and renormalise:
    raytrace_normalise(dir);

    n1 = n2;
  }

  // return [x,y,dx,dy,lambda]
  Eigen::Vector2d outpos(0, 0);
  Eigen::Vector2d outdir(0, 0);
  csToPlane(pos, dir, outpos, outdir, distsum);
  out(0) = outpos(0);
  out(1) = outpos(1);
  out(2) = outdir(0);
  out(3) = outdir(1);
  out(4) = intensity;
  
  return error;
}

// evaluate scene to sensor:
static inline int evaluate_aperture_reverse(const std::vector<lens_element_t> lenses, const int lenses_cnt, const double zoom, const Eigen::VectorXd in, Eigen::VectorXd &out, const int aspheric)
{
  int error = 0;
  double n1 = 1.0;
  Eigen::Vector3d pos(0,0,0);
  Eigen::Vector3d dir(0,0,0);
  double intensity = 1.0;

  const Eigen::Vector2d inpos(in[0], in[1]);
  const Eigen::Vector2d indir(in[2], in[3]);
  if (stringcmp(lenses[0].geometry, "cyl-y")) cylinderToCs(inpos, indir, pos, dir, 0, lenses[0].lens_radius, true);
  else if (stringcmp(lenses[0].geometry, "cyl-x")) cylinderToCs(inpos, indir, pos, dir, 0, lenses[0].lens_radius, false);
  else sphereToCs(inpos, indir, pos, dir, 0, lenses[0].lens_radius);

  for(int i = 0; i < 2; i++) dir(i) = -dir(i);

  double distsum = 0;
  for(int k=0;k<lenses_cnt;k++)
  {
    const double R = lenses[k].lens_radius;
    double t = 0.0;
    const double dist = lens_get_thickness(lenses[k], zoom);

    Eigen::Vector3d normal(0,0,0);
    error |= intersect(lenses, k, pos, dir, t, normal, R, distsum, true);

    // index of refraction and ratio current/next:
    const double n2 = spectrum_eta_from_abbe_um(lenses[k].ior, lenses[k].vno, in(4));
    intensity *= refract(n1, n2, normal, dir);
    if(intensity < INTENSITY_EPS) error |= 8;
    if(error) return error;

    // and renormalise:
    raytrace_normalise(dir);

    // move to next interface:
    distsum += dist;

    // stop after processing aperture but before moving to next element
    if(k < lenses_cnt-1 && stringcmp(lenses[k+1].material, "iris")) break;

    n1 = n2;
  }

  // return [x,y,dx,dy,lambda]
  Eigen::Vector2d outpos(0, 0);
  Eigen::Vector2d outdir(0, 0);
  csToPlane(pos, dir, outpos, outdir, distsum);
  out(0) = outpos(0);
  out(1) = outpos(1);
  out(2) = outdir(0);
  out(3) = outdir(1);
  out(4) = intensity;

  return error;
}



// line line intersection for finding the principle plane
inline double lineLineIntersection_x(const Eigen::Vector3d line1_origin, const Eigen::Vector3d line1_direction, const Eigen::Vector3d line2_origin, const Eigen::Vector3d line2_direction, const int dim_up){
    double A1 = line1_direction(dim_up) - line1_origin(dim_up);
    double B1 = line1_origin(2) - line1_direction(2);
    double C1 = A1 * line1_origin(2) + B1 * line1_origin(dim_up);
    double A2 = line2_direction(dim_up) - line2_origin(dim_up);
    double B2 = line2_origin(2) - line2_direction(2);
    double C2 = A2 * line2_origin(2) + B2 * line2_origin(dim_up);
    double delta = A1 * B2 - A2 * B1;

    return (B2 * C1 - B1 * C2) / delta;
}



// evalute sensor to outer pupil:
inline double calculate_focal_length(
      const std::vector<lens_element_t> lenses, const int lenses_cnt, 
      const double zoom, 
      const Eigen::VectorXd in, Eigen::VectorXd &out, 
      const int dim_up, const bool draw_aspherical)
{
  int error = 0;
  double n1 = spectrum_eta_from_abbe_um(lenses[lenses_cnt-1].ior, lenses[lenses_cnt-1].vno, in(4));
  Eigen::Vector3d pos(0,0,0);
  Eigen::Vector3d dir(0,0,0);
  double intensity = 1.0;

  planeToCs(Eigen::Vector2d(in(0), in(1)), Eigen::Vector2d(in(2), in(3)), pos, dir, 0);

  double distsum = 0;

  for(int k=lenses_cnt-1;k>=0;k--)
  {
    // propagate the ray reverse to the plane of intersection optical axis/lens element:
    const double R = -lenses[k].lens_radius; // negative, evaluate() is the adjoint case
    double t = 0.0;
    distsum += lens_get_thickness(lenses[k], zoom);

    //normal at intersection
    Eigen::Vector3d normal(0,0,0);
    error |= intersect(lenses, k, pos, dir, t, normal, R, distsum, true);

    // index of refraction and ratio current/next:
    const double n2 = k ? spectrum_eta_from_abbe_um(lenses[k-1].ior, lenses[k-1].vno, in(4)) : 1.0; // outside the lens there is vacuum

    intensity *= refract(n1, n2, normal, dir);
    if(intensity < INTENSITY_EPS) error |= 8;

    if(error) return error;

    raytrace_normalise(dir);

    n1 = n2;
  }
  
  // calculate focal length using principal planes
  Eigen::Vector3d pp_line1start(0.0, 0.0, 0.0);
  Eigen::Vector3d pp_line1end(0.0, 0.0, 99999.0);
  Eigen::Vector3d pp_line2end(0.0, 0.0, static_cast<double>(pos(2) + (dir(2) * 1000.0)));
  pp_line1start(dim_up) = in(dim_up);
  pp_line1end(dim_up) = in(dim_up);
  pp_line2end(dim_up) = pos(dim_up) + (dir(dim_up) * 1000.0);
  double principlePlaneDistance = lineLineIntersection_x(pp_line1start, pp_line1end, pos, pp_line2end, dim_up);

  Eigen::Vector3d focalPointLineStart(0.0, 0.0, 0.0);
  Eigen::Vector3d focalPointLineEnd(0.0, 0.0, 99999.0);
  double focalPointDistance = lineLineIntersection_x(focalPointLineStart, focalPointLineEnd, pos, pp_line2end, dim_up);

  return focalPointDistance - principlePlaneDistance;
}



static inline double evaluate_reverse_intersection_y0(
  const std::vector<lens_element_t> lenses, const int lenses_cnt, const double zoom, const Eigen::VectorXd in, Eigen::VectorXd &out, const int dim_up, const int draw_aspherical)
{
  int error = 0;
  double n1 = 1.0;
  double intensity = 1.0;

  double lens_length = 0;
  for(int i=0;i<lenses_cnt;i++) lens_length += lens_get_thickness(lenses[i], zoom);

  Eigen::Vector3d pos(0,0,0);
  Eigen::Vector3d dir(0,0,0);
  const Eigen::Vector2d inpos(in(0), in(1));
  const Eigen::Vector2d indir(in(2), in(3));
  if (stringcmp(lenses[0].geometry, "cyl-y")) cylinderToCs(inpos, indir, pos, dir, lens_length - lenses[0].lens_radius, lenses[0].lens_radius, true);
  else if (stringcmp(lenses[0].geometry, "cyl-x")) cylinderToCs(inpos, indir, pos, dir, lens_length - lenses[0].lens_radius, lenses[0].lens_radius, false);
  else sphereToCs(inpos, indir, pos, dir, lens_length - lenses[0].lens_radius, lenses[0].lens_radius);


  // sphere param only knows about directions facing /away/ from outer pupil, so
  // need to flip this if we're tracing into the lens towards the sensor:
  for(int i = 0; i < 3; i++) dir(i) = -dir(i);

  double distsum = lens_length;

  for(int k=0;k<lenses_cnt;k++)
  {
    const double R = lenses[k].lens_radius;
    double t = 0.0;
    const double dist = lens_get_thickness(lenses[k], zoom);

    //normal at intersection
    Eigen::Vector3d normal(0,0,0);
    
    error |= intersect(lenses, k, pos, dir, t, normal, R, distsum, false);

    if(normal(2) < 0.0) error |= 16;

    // index of refraction and ratio current/next:
    const double n2 = spectrum_eta_from_abbe_um(lenses[k].ior, lenses[k].vno, in(4));
    intensity *= refract(n1, n2, normal, dir);
    if(intensity < INTENSITY_EPS) error |= 8;

    //if(error) return error;

    // and renormalise:
    raytrace_normalise(dir);

    distsum -= dist;
    n1 = n2;
  }
  
  // y=0 intersection
  return pos(dim_up) - pos(2)*dir(dim_up)/dir(2);
}


static inline bool evaluate_reverse_fstop(
  const std::vector<lens_element_t> lenses, 
  const int lenses_cnt, 
  const double zoom, 
  const Eigen::VectorXd in, 
  Eigen::VectorXd &out, 
  const int dim_up, 
  const int draw_aspherical, 
  Eigen::Vector2d &positiondata,
  double &max_aperture_radius)
{
  int error = 0;
  double n1 = 1.0;
  Eigen::Vector3d pos(0,0,0);
  Eigen::Vector3d dir(0,0,0);
  double intensity = 1.0;
  const int aperture_element = lens_get_aperture_element(lenses, lenses_cnt);
  double lens_length = 0;
  for(int i=0;i<lenses_cnt;i++) lens_length += lens_get_thickness(lenses[i], zoom);

  const Eigen::Vector2d inpos(in(0), in(1));
  const Eigen::Vector2d indir(in(2), in(3));
  if (stringcmp(lenses[0].geometry, "cyl-y")) cylinderToCs(inpos, indir, pos, dir, lens_length - lenses[0].lens_radius, lenses[0].lens_radius, true);
  else if (stringcmp(lenses[0].geometry, "cyl-x")) cylinderToCs(inpos, indir, pos, dir, lens_length - lenses[0].lens_radius, lenses[0].lens_radius, false);
  else sphereToCs(inpos, indir, pos, dir, lens_length - lenses[0].lens_radius, lenses[0].lens_radius);


  // sphere param only knows about directions facing /away/ from outer pupil, so
  // need to flip this if we're tracing into the lens towards the sensor:
  for(int i = 0; i < 3; i++) dir(i) = -dir(i);

  double distsum = lens_length;

  for(int k=0;k<lenses_cnt;k++)
  {
    const double R = lenses[k].lens_radius;
    double t = 0.0;
    const double dist = lens_get_thickness(lenses[k], zoom);

    Eigen::Vector3d normal(0,0,0);
    error |= intersect(lenses, k, pos, dir, t, normal, R, distsum, false);

    if (k == aperture_element) max_aperture_radius = pos(dim_up);

    if(normal(2) < 0.0) error |= 16;

    // index of refraction and ratio current/next:
    const double n2 = spectrum_eta_from_abbe_um(lenses[k].ior, lenses[k].vno, in(4));
    intensity *= refract(n1, n2, normal, dir);
    if(intensity < INTENSITY_EPS) error |= 8;

    if(error) return false;

    // and renormalise:
    raytrace_normalise(dir);

    distsum -= dist;
    n1 = n2;
  }

  // [xdistance between last lens intersection and y0 intersection, ydistance last lens intersection]
  positiondata(0) = pos(2) - (pos(dim_up) - pos(2)*dir(dim_up)/dir(2));
  positiondata(1) = pos(dim_up);

  return true;
}