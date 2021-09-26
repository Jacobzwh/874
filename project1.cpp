
/*
 * Contains the the apply function, the cofactors and quantification functions.
 *
 * For Project 1, implement
 * (1) apply, also handles probabilities
 * (2) negative_cofactor
 * (3) positive_cofactor, 
 * (4) boolean_difference
 * (5) sort by influence
 * (6) check probability equivalence
 */
#include <vector>
#include <set>
#include <algorithm>
#include "project1.h"

using namespace std;

// wrapper function to allow calling with the operation, i.e apply(bdd1, bdd2, "or")
bdd_ptr apply(bdd_ptr bdd1, bdd_ptr bdd2, string o)
{
  operation op1;
  if (!op1.set_operation(o))
  {
    return 0;
  }
  
  return apply(bdd1, bdd2, op1);
}

// apply implements an arbitrary operation (specified in op) on two BDDs
// bdd_tables is used to handle the book keeping (see bdd_tables.h).
//
// apply works recursively one the idea that given an arbitrary operation $, 
// and functions f and g: f $ g = a'(fa' $ ga') + a(fa $ ga), 
// where a is a variable, fa' is the negative cofactor etc.
bdd_ptr apply(bdd_ptr bdd1, bdd_ptr bdd2, operation &op)
{
  // get reference to tables
  bdd_tables& tables = bdd_tables::getInstance();
  
  // ... your code goes here
  bdd_ptr node1;

  if ((node1 = tables.find_in_computed_table(op.get_operation(), bdd1, bdd2))) 
  {
      return node1;
  }

  if ((node1 = op(bdd1, bdd2))) 
  {
      return node1;
  }


  char var = find_next_var(bdd1, bdd2);

  bdd_ptr bdd1_nc=negative_cofactor(bdd1,var);
  bdd_ptr bdd1_pc=positive_cofactor(bdd1,var);
  bdd_ptr bdd2_nc=negative_cofactor(bdd2,var);
  bdd_ptr bdd2_pc=positive_cofactor(bdd2,var);

  bdd_ptr branch_l=apply(bdd1_nc,bdd2_nc,op);
  bdd_ptr branch_r=apply(bdd1_pc,bdd2_pc,op);

  
  if (branch_l==branch_r) 
  {  
      node1=branch_l;
  }
  else 
  {
      node1=tables.find_in_unique_table(var, branch_l, branch_r);

      if (node1==0) 
      {
          node1=tables.create_and_add_to_unique_table(var,branch_l,branch_r);
          node1->probability = branch_l->probability*0.5+branch_r->probability*0.5;
      }
  }

  tables.insert_computed_table(op.get_operation(), bdd1, bdd2, node1);


  // change the return value when you're finished
  return node1;

  // change the return value when you're finished
  //return NULL;
}

// negative_cofactor takes the BDD pointed to by np, 
// and returns the negative cofactor with respect to var.
bdd_ptr negative_cofactor(bdd_ptr np, char var)
{
  // get reference to tables
  bdd_tables& tables=bdd_tables::getInstance();
  
  //... your code goes here
  if (np==bdd_node::one) 
  {
      return bdd_node::one;
  }

  if (np==bdd_node::zero) 
  {
      return bdd_node::zero;
  }

  if (np->var==var) 
  {
      return np->neg_cf;
  }


  bdd_ptr branch_l=negative_cofactor(np->neg_cf,var);
  bdd_ptr branch_r=negative_cofactor(np->pos_cf,var);

  if (branch_l==branch_r)
      return branch_l;

  bdd_ptr node1;

  node1=tables.find_in_unique_table(np->var,branch_l,branch_r);

  if (node1==0) 
  {
      node1=tables.create_and_add_to_unique_table(np->var,branch_l,branch_r);
      node1->probability=branch_l->probability*0.5+branch_r->probability*0.5;
  }

  // once you add your code, remove this and return your own result
  return node1; 
 // return np; 
}

// posative_cofactor takes the BDD pointed to by np, 
// and returns the posative cofactor with respect to var.
bdd_ptr positive_cofactor(bdd_ptr np, char var)
{
  // get reference to tables
  bdd_tables& tables=bdd_tables::getInstance();
  
  //... your code goes here
    // Base case
  if (np==bdd_node::one) 
  {
      return bdd_node::one;
  }

  if (np==bdd_node::zero) 
  {
      return bdd_node::zero;
  }

  if (np->var==var) 
  {
      return np->pos_cf;
  }

  bdd_ptr branch_l = positive_cofactor(np->neg_cf,var);
  bdd_ptr branch_r = positive_cofactor(np->pos_cf,var);

  if (branch_l==branch_r)
      return branch_l;

  bdd_ptr node1;

  node1 = tables.find_in_unique_table(np->var,branch_l,branch_r);
  if (node1==0) 
  {
      node1=tables.create_and_add_to_unique_table(np->var,branch_l, branch_r);
      node1->probability=branch_l->probability*0.5+branch_r->probability*0.5;
  }

  // once you add your code, remove this and return your own result
  return node1; 
  //return np; 
}

// boolean_difference takes the BDD pointed to by np, 
// and returns the boolean difference with respect to var.
bdd_ptr boolean_difference(bdd_ptr np, char var)
{
  // get reference to tables
  bdd_tables& tables = bdd_tables::getInstance();
  
  //... your code goes here
  bdd_ptr node1 = apply(negative_cofactor(np, var), positive_cofactor(np, var), "xor");
  
  // once you add your code, remove this and return your own result
  return node1; 
  //return np; 
}

void get_vars(bdd_ptr np,set<char> &vars) 
{
    if (np->is_terminal()) 
    {
        return;
    }

    vars.insert(np->var);

    get_vars(np->neg_cf,vars);
    get_vars(np->pos_cf,vars);
}

struct infsort 
{
  
    char var;
    float inf;

    infsort(char var,float inf):var(var),inf(inf){}
};

bool islarger(infsort a,infsort b) 
{
    return (a.inf>b.inf);
}


// sort_by_influence calculates the influence of all the variables in np
// and displays them in descending order (most influent variable is
// shown first). For this task you can assume the number of variable
// is no greater than 20.
bdd_ptr sort_by_influence(bdd_ptr np)
{
  //... your code goes here
    set<char> vars;

    get_vars(np, vars);
    vector<infsort> vec1;
    for (set<char>::iterator a=vars.begin(); a!=vars.end(); ++a)
    {
      bdd_ptr node1 = boolean_difference(np, *a);
      infsort inf1(*a, node1->probability);
      vec1.push_back(inf1);
    }

    sort(vec1.begin(), vec1.end(), islarger);

    for (vector<infsort>::iterator a=vec1.begin();a!= vec1.end();++a)
    {
      cout <<a->var<<','<<a->inf<<endl;
    }
    
  // this function does not alter the current node, so np must be
  // returned at the end
  return np;
}

//check if two bdd are p equivalence, return true if they are p equivalence
//otherwise return false.
bool check_Probability_equivalence(bdd_ptr bdd1, bdd_ptr bdd2)
{
  bool is_P_equivalence = false;
  //... your code goes hereif
  //
  if (bdd1->probability==bdd2->probability)
  {
    is_P_equivalence = true;
  }
  // this function does not alter the current node, so np must be
  // returned at the end
  return is_P_equivalence;
}

