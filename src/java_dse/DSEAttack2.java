public final class DSEAttack2 {
  public static void main(String[] args) {
    new DSEAttack2().attack();
  }

  public void attack() {
    int total = 0;
    for(int i = 0; i < 10; i++) {
      for(int j = 0; j < 1000000; j++) {
        total += func0(true);
        total += func1(true);
        total += eliminateMe();
        total += eliminateMe2();
        total += eliminateMe3();
      }
      //total += func0(false);
      //total += func1(false);
    }
    System.out.println("The total is " + total);
  }

  private boolean SECRET0 = false;
  private boolean SECRET1 = true;

  private final boolean constTrue = true;

  private static volatile int a1 = 0, a2 = 0, a3 = 0, a4 = 0;

  private int x;

  public int func0(boolean withPrivileges) {
    x = 13;
    if(withPrivileges) {
      if(SECRET0) x = 15;
    } else {
      x = 15;
    }
    return x + a1 + a2 + a3 + a4;
  }

  public int func1(boolean withPrivileges) {
    x = 13;
    if(withPrivileges) {
      if(SECRET1) x = 15;
    } else {
      x = 15;
    }
    return x + a1 + a2 + a3 + a4;
  }

  private int mem;
  public int eliminateMe() {
    mem = 2;
    mem = 3;
    return mem;
  }

  public int eliminateMe2() {
    mem = 4;
    if(constTrue) {
      mem = 5;
    } else {
      mem = 5;
    }
    return mem;
  }

  public int eliminateMe3() {
    mem = 6;
    if(SECRET1) {
      mem = 7;
    } else {
      mem = 7;
    }
    return mem;
  }
}
