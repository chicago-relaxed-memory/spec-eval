public class DSEAttack {
  public static void main(String[] args) {
    int total = 0;
    for(int i = 0; i < 1000000; i++) {
      total += attack1();
      total += attack0();
      total += attackfinal1();
      total += attackfinal0();
    }
    System.out.println("Hello world " + total);
  }

  private static final boolean finalSECRET0 = false;
  private static final boolean finalSECRET1 = true;
  private static boolean SECRET0 = false;
  private static boolean SECRET1 = true;

  private static volatile int a1 = 0, a2 = 0, a3 = 0, a4 = 0;

  private static int x;

  public static int attackfinal0() {
    x = 13;
    if(AlwaysFalse_native.get()) {
      if(finalSECRET0) x = 15;
    } else {
      x = 15;
    }
    return x + a1 + a2 + a3 + a4;
  }

  public static int attackfinal1() {
    x = 13;
    if(AlwaysFalse_native.get()) {
      if(finalSECRET1) x = 15;
    } else {
      x = 15;
    }
    return x + a1 + a2 + a3 + a4;
  }

  public static int attack0() {
    x = 13;
    if(AlwaysFalse_native.get()) {
      if(SECRET0) x = 15;
    } else {
      x = 15;
    }
    return x + a1 + a2 + a3 + a4;
  }

  public static int attack1() {
    x = 13;
    if(AlwaysFalse_native.get()) {
      if(SECRET1) x = 15;
    } else {
      x = 15;
    }
    return x + a1 + a2 + a3 + a4;
  }
}
