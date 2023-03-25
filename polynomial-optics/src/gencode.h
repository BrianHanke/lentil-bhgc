#pragma once
#include "poly.h"
#include "../../Eigen/Eigen/Dense"


void print_poly_system_code(FILE *f, const poly_system_t *system,
    const char *vnamei[poly_num_vars],
    const char *vnameo[poly_num_vars],
    const char *case_lens_name)
{
  fprintf(f, "case %s:\n{\n", case_lens_name);

  for(int i = 0; i < poly_num_vars - 1; i++)
  {
    fprintf(f, "out[%d] = ", i);
    poly_print(&system->poly[i], vnamei, f);
    fprintf(f, ";\n");
  }
  fprintf(f, "out_transmittance = ");
  poly_print(&system->poly[4], vnamei, f);
  fprintf(f, ";\n");

  fprintf(f, "} break;\n");
}

// dump out jacobian source code to a stream
void print_jacobian(FILE *f, const poly_system_t *system, const char *vnamei[poly_num_vars], const char *case_lens_name)
{
  fprintf(f, "case %s:\n{\n", case_lens_name);
  poly_jacobian_t jacobian;
  poly_system_get_jacobian(system, &jacobian);
  for(int i = 0; i < poly_num_vars; i++)
    for(int j = 0; j < poly_num_vars; j++)
    {
      fprintf(f, "const double dx%d%d = ", i, j);
      poly_print(&jacobian.poly[i*poly_num_vars+j], vnamei, f);
      fprintf(f, "+0.0f;\n");
    }
  fprintf(f, "} break;\n");
}

void print_pt_sample_aperture(FILE *f, const poly_system_t *system,
    const char *vnamei[poly_num_vars],
    const char *vnameo[poly_num_vars], 
    const char *case_lens_name)
{
  fprintf(f, "case %s:\n{\n", case_lens_name);

  // input to this is [x,y,dx,dy] and `dist', which is the distance to the start of the polynomial.

  char *begin_var[poly_num_vars];
  for(int k=0;k<poly_num_vars;k++) begin_var[k] = static_cast<char *>(malloc(50));
  for(int k=0;k<poly_num_vars;k++) snprintf(begin_var[k], 50, "begin_%s", vnamei[k]);

  // solve the first two rows of the system x1 = P(x0, omega0, ..) for given x1 and x0.
  // 1) first guess, take omega0 = normalise(x1 - x0), leave rest of x0 input unchanged
  //    this has to be done outside, we don't know about the 3d geometry any more.
  // double sqr_err = 0.0f;
  for(int k=0;k<4;k++)
    fprintf(f, "double pred_%s;\n", vnameo[k]);
  fprintf(f, "double sqr_err = FLT_MAX;\n");
  fprintf(f, "for(int k=0;k<5&&sqr_err > 1e-4f;k++)\n{\n");

  // 1.5) compute input to the polynomial, begin_[x,y,dx,dy] from our initial guess and the distance:
  fprintf(f, "  const double %s = %s + dist * %s;\n", begin_var[0], vnamei[0], vnamei[2]);
  fprintf(f, "  const double %s = %s + dist * %s;\n", begin_var[1], vnamei[1], vnamei[3]);
  fprintf(f, "  const double %s = %s;\n", begin_var[2], vnamei[2]);
  fprintf(f, "  const double %s = %s;\n", begin_var[3], vnamei[3]);
  // wavelength may be unused for lenses such as lensbaby, where the aperture comes right after
  // the sensor, without glass elements in between:
  fprintf(f, "  const double %s = %s;\n", begin_var[4], vnamei[4]);

  // 2) evaluate what we get x1' = P(x0, omega0, ..)
  for(int k=0;k<4;k++)
  {
    fprintf(f, "  pred_%s = ", vnameo[k]);
    poly_print(&system->poly[k], (const char**)begin_var, f);
    fprintf(f, ";\n");
  }

  // 3) evaluate 2x2 submatrix of jacobian dx1/domega0 and step omega0 back to our target x1.
  poly_jacobian_t sysjac;
  poly_system_get_jacobian(system, &sysjac);
  fprintf(f, "  Eigen::Matrix2d dx1_domega0;\n");
  for(int i=0;i<2;i++) for(int j=0;j<2;j++)
  {
    fprintf(f, "  dx1_domega0(%d, %d) = ", i, j);
    poly_print(&sysjac.poly[i*poly_num_vars+j+2], (const char**)begin_var, f);
    fprintf(f, "+0.0f;\n");
  }
  for(int k=0;k<poly_num_vars*poly_num_vars;k++)
    poly_destroy(sysjac.poly+k);

  // 4) invert jacobian (could use adjoint, but who's gonna fight over a 2x2 inversion)
  fprintf(f, "  Eigen::Matrix2d invJ;\n");
  fprintf(f, "  const double invdet = 1.0f/(dx1_domega0(0, 0)*dx1_domega0(1, 1) - dx1_domega0(0, 1)*dx1_domega0(1, 0));\n");
  fprintf(f, "  invJ(0, 0) =  dx1_domega0(1, 1)*invdet;\n");
  fprintf(f, "  invJ(1, 1) =  dx1_domega0(0, 0)*invdet;\n");
  fprintf(f, "  invJ(0, 1) = -dx1_domega0(0, 1)*invdet;\n");
  fprintf(f, "  invJ(1, 0) = -dx1_domega0(1, 0)*invdet;\n");

  // 5) determine step and update omega0
  fprintf(f, "  const Eigen::Vector2d dx1(out_x - pred_x, out_y - pred_y);\n");
    // sqr_err = 0.0f;
    // for(int k=0;k<2;k++) sqr_err += dx1[k]*dx1[k];
  fprintf(f, "  for(int i=0;i<2;i++)\n  {\n");
  for(int k=0;k<2;k++)
  fprintf(f, "    %s += invJ(%d, i)*dx1(i);\n", vnamei[k+2], k);
  fprintf(f, "  }\n");
  fprintf(f, "  sqr_err = dx1(0)*dx1(0) + dx1(1)*dx1(1);\n");

  fprintf(f, "}\n"); // end newton iteration loop
  // now evaluate omega out
  for(int k=0;k<2;k++)
    fprintf(f, "out_%s = pred_%s;\n", vnameo[k+2], vnameo[k+2]);
  for(int k=0;k<poly_num_vars;k++) free(begin_var[k]);

  fprintf(f, "} break;\n");
}

// light tracer connecting to a given point on the aperture and in the scene.
// idea as follows:
//
// evaluate:
//
// input : z[x,y,z] point on the scene geometry
// input : a[x,y,u,v] point on aperture, [u,v] initial guess
// output: x[x,y,u,v] point on the sensor
//
// // do newton iterations
// while(error > EPSILON || apertureError > EPSILON)
// {
//   // calculate error in aperture position
//   delta_ap = (a - Pa(x))[x,y]
//   // propagate back error to sensor direction through 2x2 subblock of jacobian
//   // mapping sensor directions to aperture positions
//   x[u,v] += Ja^{-1}(x)*delta_ap
//   // calculate error in direction on the outer pupil
//   out = fromSpherical(P(x))[x,y,z,u,v,w]
//   reqDir = normalise(z - out[x,y,z])
//   delta_out = toSpherical(out[u,v,w]-reqDir)
//   // propagate back error to sensor position through 2x2 subblock of jacobian
//   // mapping sensor positions to outer pupil directions
//   x[x,y] += J^{-1}(x) * delta_out
// }
void print_lt_sample_aperture(FILE *f, const poly_system_t *system, const poly_system_t *ap_system,
    const char *vnamei[poly_num_vars],
    const char *vnameo[poly_num_vars], 
    const char *case_lens_name)
{
  fprintf(f, "case %s:\n{\n", case_lens_name);

  // input to this is scene_[x,y,z] point in scene and ap_[x,y] point on the aperture
  fprintf(f, "//input: scene_[x,y,z] - point in scene, ap_[x,y] - point on aperture\n");
  fprintf(f, "//output: [x,y,dx,dy] point and direction on sensor\n");
  //for debug output
  //fprintf(f, "#ifndef DEBUG_LOG\n#define DEBUG_LOG\n#endif\n");

  char *begin_var[poly_num_vars];
  for(int k=0;k<poly_num_vars;k++) begin_var[k] = static_cast<char *>(malloc(50));
  for(int k=0;k<poly_num_vars;k++) snprintf(begin_var[k], 50, "begin_%s", vnamei[k]);
  //early out if worldspace point is definitely outside field of view:
  fprintf(f, "Eigen::Vector3d view(\n");
  fprintf(f, "  scene_x,\n");
  fprintf(f, "  scene_y,\n");
  fprintf(f, "  scene_z + lens_outer_pupil_curvature_radius\n);\n");
  fprintf(f, "raytrace_normalise(view);\n");
  fprintf(f, "int error = 0;\n");
  fprintf(f, "if(1 || view(2) >= lens_field_of_view)\n{\n");
  fprintf(f, "  const double eps = 1e-8;\n");
  fprintf(f, "  double sqr_err = 1e30, sqr_ap_err = 1e30;\n");
  fprintf(f, "  double prev_sqr_err = 1e32, prev_sqr_ap_err = 1e32;\n");
  //also stop if error is getting larger
  fprintf(f, "  for(int k=0;k<100&&(sqr_err>eps||sqr_ap_err>eps)&&error==0;k++)\n  {\n");
  fprintf(f, "    prev_sqr_err = sqr_err, prev_sqr_ap_err = sqr_ap_err;\n");

  // 1) input to the polynomial, begin_[x,y,dx,dy], is always the same as the current sensor guess vname:
  fprintf(f, "    const double %s = %s;\n", begin_var[0], vnamei[0]);
  fprintf(f, "    const double %s = %s;\n", begin_var[1], vnamei[1]);
  fprintf(f, "    const double %s = %s;\n", begin_var[2], vnamei[2]);
  fprintf(f, "    const double %s = %s;\n", begin_var[3], vnamei[3]);
  fprintf(f, "    const double %s = %s;\n", begin_var[4], vnamei[4]);

  // 2) evaluate aperture position and calculate error vector
  fprintf(f, "    const Eigen::Vector2d pred_ap(\n");
  for(int k=0;k<2;k++)
  {
    fprintf(f, "      ");
    poly_print(&ap_system->poly[k], (const char**)begin_var, f);
    fprintf(f, "%s", k<1?",\n":"\n    );\n");
  }

  fprintf(f, "    const Eigen::Vector2d delta_ap(ap_%s - pred_ap[0], ap_%s - pred_ap[1]);\n", vnameo[0], vnameo[1]);
  fprintf(f, "    sqr_ap_err = delta_ap[0]*delta_ap[0]+delta_ap[1]*delta_ap[1];\n");

  // 3) calculate inverse 2x2 submatrix of jacobian and propagate error back to sensor direction
  // 3.1) evaluate aperture jacobian
  poly_jacobian_t apjac;
  poly_system_get_jacobian(ap_system, &apjac);
  fprintf(f, "    Eigen::Matrix2d dx1_domega0;\n");
  for(int i=0;i<2;i++) for(int j=0;j<2;j++)
  {
    fprintf(f, "    dx1_domega0(%d, %d) = ", i, j);
    poly_print(&apjac.poly[i*poly_num_vars+j+2], (const char**)begin_var, f);
    fprintf(f, "+0.0f;\n");
  }

  // 3.2) invert jacobian, leaving manual way here just for future reference on how to find the determinant
  // fprintf(f, "    Eigen::Matrix2d invApJ;\n");
  // fprintf(f, "    const double invdetap = 1.0f/(dx1_domega0(0, 0)*dx1_domega0(1, 1) - dx1_domega0(0, 1)*dx1_domega0(1, 0));\n");
  // fprintf(f, "    invApJ(0, 0) =  dx1_domega0(1, 1)*invdetap;\n");
  // fprintf(f, "    invApJ(1, 1) =  dx1_domega0(0, 0)*invdetap;\n");
  // fprintf(f, "    invApJ(0, 1) = -dx1_domega0(0, 1)*invdetap;\n");
  // fprintf(f, "    invApJ(1, 0) = -dx1_domega0(1, 0)*invdetap;\n");
  fprintf(f, "    Eigen::Matrix2d invApJ = dx1_domega0.inverse().eval();\n");

  // 3.3) propagate back error
  // We do not need to check if the error is small here, as we would have exited
  // the loop if the error had been small
  // fprintf(f, "    for(int i=0;i<2;i++)\n    {\n");
  // for(int k=0;k<2;k++)
  // {
  //   fprintf(f, "      %s += invApJ(%d, i)*delta_ap[i];\n", vnamei[k+2], k);
  // }
  // fprintf(f, "    }\n");
  fprintf(f, "    Eigen::Vector2d solution_dir = invApJ * delta_ap;\n");
  fprintf(f, "    dx += solution_dir(0);\n");
  fprintf(f, "    dy += solution_dir(1);\n");


  // 4) evaluate scene direction and calculate error vector
  // - get out position and out direction in worldspace
  // - calculate vector from out position to scene point
  // - transform this vector back to spherical coordinates
  
  //do not calculate transmittance all the time, just once at the end
  for(int k=0;k<4;k++)
  {
    fprintf(f, "    out(%d) = ", k);
    poly_print(&system->poly[k], (const char**)begin_var, f);
    fprintf(f, ";\n");
  }

  fprintf(f, "    Eigen::Vector3d pred_out_cs_pos(0,0,0);\n");
  fprintf(f, "    Eigen::Vector3d pred_out_cs_dir(0,0,0);\n");
  fprintf(f, "    Eigen::Vector2d outpos(out(0), out(1));\n");
  fprintf(f, "    Eigen::Vector2d outdir(out(2), out(3));\n");

  fprintf(f, "    if (lens_outer_pupil_geometry == \"cyl-y\") cylinderToCs(outpos, outdir, pred_out_cs_pos, pred_out_cs_dir, - lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius, true);\n");
	fprintf(f, "    else if (lens_outer_pupil_geometry == \"cyl-x\") cylinderToCs(outpos, outdir, pred_out_cs_pos, pred_out_cs_dir, - lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius, false);\n");
  fprintf(f, "    else sphereToCs(outpos, outdir, pred_out_cs_pos, pred_out_cs_dir, - lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius);\n");

  fprintf(f, "    Eigen::Vector3d view(\n");
  fprintf(f, "      scene_x - pred_out_cs_pos(0),\n");
  fprintf(f, "      scene_y - pred_out_cs_pos(1),\n");
  fprintf(f, "      scene_z - pred_out_cs_pos(2)\n    );\n");
  fprintf(f, "    raytrace_normalise(view);\n");

  fprintf(f, "    Eigen::VectorXd out_new(5); out_new.setZero();\n");
  fprintf(f, "    Eigen::Vector2d out_new_pos(0,0);\n");
  fprintf(f, "    Eigen::Vector2d out_new_dir(0,0);\n");

  //Position is just converted back, direction gets replaced with new one
  fprintf(f, "    if (lens_outer_pupil_geometry == \"cyl-y\") csToCylinder(pred_out_cs_pos, view, out_new_pos, out_new_dir, - lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius, true);\n");
	fprintf(f, "    else if (lens_outer_pupil_geometry == \"cyl-x\") csToCylinder(pred_out_cs_pos, view, out_new_pos, out_new_dir, - lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius, false);\n");
  fprintf(f, "    else csToSphere(pred_out_cs_pos, view, out_new_pos, out_new_dir, - lens_outer_pupil_curvature_radius, lens_outer_pupil_curvature_radius);\n");
  fprintf(f, "    out_new(0) = out_new_pos(0);\n");
  fprintf(f, "    out_new(1) = out_new_pos(1);\n");
  fprintf(f, "    out_new(2) = out_new_dir(0);\n");
  fprintf(f, "    out_new(3) = out_new_dir(1);\n");

  //Calculate error vector (out_new - pred_out)[dx,dy]
  fprintf(f, "    const Eigen::Vector2d delta_out(out_new[2] - out[2], out_new[3] - out[3]);\n");
  fprintf(f, "    sqr_err = delta_out[0]*delta_out[0]+delta_out[1]*delta_out[1];\n");

  // 5) Propagate error back to sensor position
  // 5.1) calculate inverse 2x2 submatrix of jacobian mapping sensor positions to outgoing directions
  poly_jacobian_t jac;
  poly_system_get_jacobian(system, &jac);
  fprintf(f, "    Eigen::Matrix2d domega2_dx0;\n");
  for(int i=0;i<2;i++) for(int j=0;j<2;j++)
  {
    fprintf(f, "    domega2_dx0(%d, %d) = ", i, j);
    poly_print(&jac.poly[(i+2)*poly_num_vars+j], (const char**)begin_var, f);
    fprintf(f, "+0.0f;\n");
  }

  // 5.2) invert jacobian
  // fprintf(f, "    Eigen::Matrix2d invJ;\n");
  // fprintf(f, "    const double invdet = 1.0f/(domega2_dx0(0, 0)*domega2_dx0(1, 1) - domega2_dx0(0, 1)*domega2_dx0(1, 0));\n");
  // fprintf(f, "    invJ(0, 0) =  domega2_dx0(1, 1)*invdet;\n");
  // fprintf(f, "    invJ(1, 1) =  domega2_dx0(0, 0)*invdet;\n");
  // fprintf(f, "    invJ(0, 1) = -domega2_dx0(0, 1)*invdet;\n");
  // fprintf(f, "    invJ(1, 0) = -domega2_dx0(1, 0)*invdet;\n");
  fprintf(f, "    Eigen::Matrix2d invJ = domega2_dx0.inverse().eval();\n");

  // 5.3) propagate error back to sensor
  // fprintf(f, "    for(int i=0;i<2;i++)\n    {\n");
  // for(int k=0;k<2;k++)
  //   fprintf(f, "      %s += 0.72 * invJ(%d, i) * delta_out[i];\n", vnamei[k], k); //note the magic .72 number, dampening the newton iterations
  // fprintf(f, "    }\n");
  fprintf(f, "    Eigen::Vector2d solution_pos = 0.72 * invJ * delta_out; // default newton-raphson\n");
  fprintf(f, "    x += solution_pos(0);\n");
  fprintf(f, "    y += solution_pos(1);\n");
  
  fprintf(f, "    if(sqr_err>prev_sqr_err) error |= 1;\n");
  fprintf(f, "    if(sqr_ap_err>prev_sqr_ap_err) error |= 2;\n");
  fprintf(f, "    if(out[0]!=out[0]) error |= 4;\n");
  fprintf(f, "    if(out[1]!=out[1]) error |= 8;\n");
  //fprintf(f, "    if(out[0]*out[0]+out[1]*out[1] > lens_outer_pupil_radius*lens_outer_pupil_radius) error |= 16;\n");
  //fprintf(f, "    DEBUG_LOG;\n");
  fprintf(f, "    // reset error code for first few iterations.\n");
  fprintf(f, "    if(k<10) error = 0;\n");
  fprintf(f, "  }\n");
  fprintf(f, "}\nelse\n  error = 128;\n");
  fprintf(f, "if(out[0]*out[0]+out[1]*out[1] > lens_outer_pupil_radius*lens_outer_pupil_radius) error |= 16;\n");
  
  //now calculate transmittance or set it to zero if we stoped due to divergence
  fprintf(f, "const double %s = %s;\n", begin_var[0], vnamei[0]);
  fprintf(f, "const double %s = %s;\n", begin_var[1], vnamei[1]);
  fprintf(f, "const double %s = %s;\n", begin_var[2], vnamei[2]);
  fprintf(f, "const double %s = %s;\n", begin_var[3], vnamei[3]);
  fprintf(f, "const double %s = %s;\n", begin_var[4], vnamei[4]);
  
  fprintf(f, "if(error==0)\n");
  fprintf(f, "  out[4] = ");
  poly_print(&system->poly[4], (const char**)begin_var, f);
  fprintf(f, ";\n");
  fprintf(f, "else\n  out[4] = 0.0f;\n");
  for(int k=0;k<poly_num_vars;k++) free(begin_var[k]);

  fprintf(f, "} break;\n");
}
