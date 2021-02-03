use std::env; // to get arugments passed to the program
use std::thread; // for the threads
//use thread::JoinHandle;

/*
* Print the number of partitions and the size of each partition
* @param vs A vector of vectors
*/
fn print_partition_info(vs: &Vec<Vec<usize>>){
    println!("Number of partitions = {}", vs.len());
    for i in 0..vs.len(){
        println!("\tsize of partition {} = {}", i, vs[i].len());
    }
}

/*
* Create a vector with integers from 0 to num_elements -1
* @param num_elements How many integers to generate
* @return A vector with integers from 0 to (num_elements - 1)
*/
fn generate_data(num_elements: usize) -> Vec<usize>{
    let mut v : Vec<usize> = Vec::new();
    for i in 0..num_elements {
        v.push(i);
    }
    return v;
}

/*
* Partition the data in the vector v into 2 vectors
* @param v Vector of integers
* @return A vector that contains 2 vectors of integers
*/

fn partition_data_in_two(v: &Vec<usize>) -> Vec<Vec<usize>>{
    let partition_size = v.len() / 2;
    // Create a vector that will contain vectors of integers
    let mut xs: Vec<Vec<usize>> = Vec::new();

    // Create the first vector of integers
    let mut x1 : Vec<usize> = Vec::new();
    // Add the first half of the integers in the input vector to x1
    for i in 0..partition_size{
        x1.push(v[i]);
    }
    // Add x1 to the vector that will be returned by this function
    xs.push(x1);

    // Create the second vector of integers
    let mut x2 : Vec<usize> = Vec::new();
    // Add the second half of the integers in the input vector to x2
    for i in partition_size..v.len(){
        x2.push(v[i]);
    }
    // Add x2 to the vector that will be returned by this function
    xs.push(x2);
    // Return the result vector
    xs
}

/*
* Sum up the all the integers in the given vector
* @param v Vector of integers
* @return Sum of integers in v
* Note: this function has the same code as the reduce_data function.
*       But don't change the code of map_data or reduce_data.
*/
fn map_data(v: &Vec<usize>) -> usize{
    let mut sum = 0;
    for i in v{
        sum += i;
    }
    sum
}

/*
* Sum up the all the integers in the given vector
* @param v Vector of integers
* @return Sum of integers in v
*/
fn reduce_data(v: &Vec<usize>) -> usize{
    let mut sum = 0;
    for i in v{
        sum += i;
    }
    sum
}
fn square(x : &usize) -> usize {
  x*x
}
fn str_len(x : String) -> usize {
  x.len()
}
/*
* A single threaded map-reduce program
*/
fn main() {
  let s = String::from("Hello");
    let len = str_len(s);
      println!("Length of {} is {}", s, len);

  let x = 10;
  square(&x);
  let username = "foo";
  let username = "bar";
  println!("here: {}", username);

    // Use std::env to get arguments passed to the program
    let args: Vec<String> = env::args().collect();
    if args.len() != 3 {
        println!("ERROR: Usage {} num_partitions num_elements", args[0]);
        return;
    }
    let num_partitions : usize = args[1].parse().unwrap();
    let num_elements : usize = args[2].parse().unwrap();
    if num_partitions < 1{
      println!("ERROR: num_partitions must be at least 1");
        return;
    }
    if num_elements < num_partitions{
        println!("ERROR: num_elements cannot be smaller than num_partitions");
        return;
    }

    // Generate data.
    let v = generate_data(num_elements);

    // PARTITION STEP: partition the data into 2 partitions
    let xs = partition_data_in_two(&v);

    // Print info about the partitions
    print_partition_info(&xs);

    let mut intermediate_sums : Vec<usize> = Vec::new();

    // MAP STEP: Process each partition

    // CHANGE CODE START: Don't change any code above this line

    let clone_a = xs[0].clone();
    let clone_b = xs[1].clone();
    //Creates vector to hold the thread handles to join later
    let handle = thread::spawn(move || map_data(&clone_a)); //Creates the new thread to handle the clonned partition
    let handle_two = thread::spawn(move || map_data(&clone_b)); //Creates the new thread to handle the clonned partition

    intermediate_sums.push(handle.join().unwrap());
    intermediate_sums.push(handle_two.join().unwrap());
    
    // CHANGE CODE END: Don't change any code below this line until the next CHANGE CODE comment

    // Print the vector with the intermediate sums
    println!("Intermediate sums = {:?}", intermediate_sums);

    // REDUCE STEP: Process the intermediate result to produce the final result
    let mut sum = reduce_data(&intermediate_sums);
    println!("Sum = {}", sum);
    
    // CHANGE CODE: Add code that does the following:
    // 1. Calls partition_data to partition the data into equal partitions
    // 2. Calls print_partition_info to print info on the partitions that have been created
    // 3. Creates one thread per partition and uses each thread to concurrently process one partition
    // 4. Collects the intermediate sums from all the threads
    // 5. Prints information about the intermediate sums
    // 5. Calls reduce_data to process the intermediate sums
    // 6. Prints the final sum computed by reduce_data

    let js = partition_data(num_partitions, &v);
    //println!("{:?}", js);
    // Print info about the partitions
    print_partition_info(&js);
    let mut intermediate_sums_two : Vec<usize> = Vec::new();
    let mut thread_list = Vec::new();
    
    for i in 0..js.len() {
      let par_clone = js[i].clone();				//Clones the partition
      let handle = thread::spawn(move || map_data(&par_clone)); //Creates the new thread to handle the clonned partition
      thread_list.push(handle);					//Adds the thread handle to join later
      //intermediate_sums.push(map_data(&par_clone));
    }

    //Iterates through the threads and join all of them and add their results to the intermediate sums list
    for handle in thread_list {
      let res = handle.join().unwrap();
      intermediate_sums_two.push(res);
    }

    //intermediate_sums.push(map_data(&xs[0]));
    //intermediate_sums.push(map_data(&xs[1]));

    println!("Intermediate sums = {:?}", intermediate_sums_two);

    // REDUCE STEP: Process the intermediate result to produce the final result
    sum = reduce_data(&intermediate_sums_two);
    println!("Sum = {}", sum);
}

/*
* CHANGE CODE: code this function
* Note: Don't change the signature of this function
*
* Partitions the data into a number of partitions such that
* - the returned partitions contain all elements that are in the input vector
* - if num_elements is a multiple of num_partitions, then all partitions must have equal number of elements
* - if num_elements is not a multiple of num_partitions, some partitions can have one more element than other partitions
*
* @param num_partitions The number of partitions to create
* @param v The data to be partitioned
* @return A vector that contains vectors of integers
* 
*/
fn partition_data(num_partitions: usize, v: &Vec<usize>) -> Vec<Vec<usize>>{
  let partition_size = v.len() / num_partitions;
  let mut xs: Vec<Vec<usize>> = Vec::new();             //2D vector holds the partition of integers
  let mut par_start = 0;				//Each time it loops this tells the partition which part of the origional vector to read from 
  let mut par_dif;					//Used to incrament par_start
  let mut par_end = partition_size;			//Where the partition will stop reading from the origional vector
  let mut extra_v = v.len() % num_partitions;		//Used to determine how many of the partitions will have an extra variable


    for _i in 0..num_partitions {
      //Creates the new parititon entry
      let mut temp_vec : Vec<usize> = Vec::new();

      //Makes some partitions bigger
      if extra_v > 0 {
        par_end += 1;
	extra_v -= 1;
      }
      //println!("parStart: {}, parEnd: {}, parDif: {}, extraV: {}", parStart, parEnd, parDif, extraV);

      //Adds the values to the partitions
      for j in par_start..par_end {
        temp_vec.push(v[j]);

      }

      //Adds partition to the 2D vector
      xs.push(temp_vec);
      par_dif = par_end - par_start;//how many items were counted is added to where the next partition should start
      par_start += par_dif;
      par_end += partition_size;
    }

    xs
}
