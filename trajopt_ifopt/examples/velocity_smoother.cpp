#include <iostream>

#include <ifopt/problem.h>
#include <ifopt/ipopt_solver.h>

#include <trajopt_ifopt/constraints/joint_position_constraint.h>
#include <trajopt_ifopt/constraints/joint_velocity_constraint.h>
#include <trajopt_ifopt/variable_sets/joint_position_variable.h>
#include <trajopt_ifopt/costs/squared_cost.h>

int main(int argc, char** argv)
{
  // 1) Create the problem
  ifopt::Problem nlp;

  // 2) Add Variables
  std::vector<trajopt::JointPosition::Ptr> vars;
  for (int ind = 0; ind < 9; ind++)
  {
    auto pos = Eigen::VectorXd::Ones(7);
    auto var = std::make_shared<trajopt::JointPosition>(pos, "Joint_Position_" + std::to_string(ind));
    vars.push_back(var);
    nlp.AddVariableSet(var);
  }

  // 3) Add constraints
  Eigen::VectorXd start_pos(7);
  start_pos << 0, 0, 0, 0, 0, 0, 0;
  std::vector<trajopt::JointPosition::Ptr> start;
  start.push_back(vars.front());
  auto start_constraint = std::make_shared<trajopt::JointPosConstraint>(start_pos, start, "StartPosition");
  nlp.AddConstraintSet(start_constraint);

  Eigen::VectorXd end_pos(7);
  end_pos << 1, 1, 1, 1, 1, 1, 1;
  std::vector<trajopt::JointPosition::Ptr> end;
  end.push_back(vars.back());
  auto end_constraint = std::make_shared<trajopt::JointPosConstraint>(end_pos, end, "EndPosition");
  nlp.AddConstraintSet(end_constraint);

  // 4) Add costs
  Eigen::VectorXd vel_target(7);
  vel_target << 0, 0, 0, 0, 0, 0, 0;
  auto vel_constraint = std::make_shared<trajopt::JointVelConstraint>(vel_target, vars, "JointVelocity");

  // Must link the variables to the constraint since that happens in AddConstraintSet
  vel_constraint->LinkWithVariables(nlp.GetOptVariables());
  auto vel_cost = std::make_shared<trajopt::SquaredCost>(vel_constraint);
  nlp.AddCostSet(vel_cost);

  nlp.PrintCurrent();
  std::cout << "Constraint Jacobian: \n" << nlp.GetJacobianOfConstraints() << std::endl;

  std::cout << "Cost Jacobian: \n" << nlp.GetJacobianOfCosts() << std::endl;

  // 5) choose solver and options
  ifopt::IpoptSolver ipopt;
  ipopt.SetOption("derivative_test", "first-order");
  ipopt.SetOption("linear_solver", "mumps");
  //  ipopt.SetOption("jacobian_approximation", "finite-difference-values");
  ipopt.SetOption("jacobian_approximation", "exact");
  ipopt.SetOption("print_level", 5);

  // 6) solve
  ipopt.Solve(nlp);
  Eigen::VectorXd x = nlp.GetOptVariables()->GetValues();
  std::cout << x.transpose() << std::endl;

  nlp.PrintCurrent();
}