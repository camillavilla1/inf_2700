
SELECT PaymentsAb.customerNumber, AmountOrdered.TotalPrice AS TotalPrice, PaymentsAb.SumAmount AS SumAmount
FROM
	(SELECT O.customerNumber, O.orderNumber, OD.orderNumber, SUM(OD.quantityOrdered * OD.priceEach) AS TotalPrice
	FROM Orders O, OrderDetails OD
		ON O.orderNumber = OD.orderNumber
		GROUP BY O.customerNumber) AS AmountOrdered
	INNER JOIN
	(SELECT P.customerNumber, SUM(P.amount) AS SumAmount
	FROM Payments P
	GROUP BY P.customerNumber) AS PaymentsAb
		ON AmountOrdered.customerNumber = PaymentsAb.customerNumber
	WHERE AmountOrdered.TotalPrice - PaymentsAb.SumAmount >= 0.1;